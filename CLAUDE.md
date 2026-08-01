# CLAUDE.md — Deep Pricer

Neural-network surrogate for a Monte Carlo exotic-derivatives pricer. C++ numerical core,
Python training layer, ONNX-served FastAPI risk service.

**Read `docs/spec.md` (what we're building) and `docs/build-order.md` (the order and the
validation gate for each step) before proposing work.** Do not skip ahead in the build
order: each step's ground-truth check must pass before the next step begins.

---

## Governing principle

**Never work without ground truth.** Every change ends in a number checked against
something already trusted — an analytic formula, a converged MC price, an independent
implementation, or a previously recorded benchmark. A change that cannot be validated
this way is not finished, no matter how clean the code looks.

Corollaries:

- Do not "fix" a numerical discrepancy by loosening a tolerance. Diagnose it. Report the
  reproducing parameters.
- Discretisation bias is expected and is not a bug — quantify it, document it, correct it
  deliberately (see the barrier/Brownian-bridge work in Step 4).
- Never report a speedup without the accuracy cost alongside it.
- Never report surrogate accuracy without stating the reference (converged MC with its
  standard error, or closed form) and the domain it holds over.

---

## Ownership boundary — IMPORTANT

I (Matt) hand-write the mathematics and the performance-critical paths. This is a
portfolio project whose purpose is to demonstrate that *I* can build this, and I must be
able to defend every design decision under interview questioning.

**Do not write or modify these without me explicitly asking in the current session:**

- `cpp/mc/` — the Monte Carlo engine, path generation, discretisation schemes
- `cpp/payoffs/` — payoff definitions and their pathwise sensitivities
- `cpp/adjoints/` — pathwise adjoint / AAD code
- `python/training/losses.py` — the differential-ML loss and its weighting
- Anything implementing a numerical scheme, variance reduction, or a Greek

For these, act as a **reviewer**: read my implementation, question the maths, point at
suspected sign errors or chain-rule mistakes, suggest optimisations with a rationale, and
propose benchmarks that would prove or disprove them. Suggest; don't substitute.

**You own the surface area around it** — please do take the initiative here:

- CMake, toolchain config, GitHub Actions, lint/sanitizer setup
- pybind11 bindings and NumPy zero-copy glue
- dataset generation plumbing, Parquet I/O, manifest generation
- the FastAPI service, ONNX runtime wiring, Dockerfile, compose
- test scaffolding and fixtures (the *harness*, not the expected numerical values)
- the agent harnesses in `agents/` (validation, experiment sweeps, benchmark reports)
- docs, README structure, figure generation scripts

## Workflow

- **Work on a branch, never on `main`.** Open a PR-shaped summary of what changed and
  why; I review before merge.
- CI must be green before you consider a task done: build with sanitizers, tests, lint,
  and the end-to-end smoke.
- Small, reviewable commits with real messages. No mega-commits mixing plumbing with
  numerics.
- If a task needs a decision I haven't made (which scheme, which tolerance, which
  library), stop and ask rather than picking one silently.
- Every agent-generated artefact gets committed — sweep summaries to `experiments/`,
  validation reports to `reports/`. Invisible agent activity is worthless to me; the
  audit trail is part of the deliverable.

## Testing standards

- Analytic tests: exact expected values, tight tolerances.
- Statistical tests: assert against MC standard error (e.g. within 3 SE), never against a
  hard-coded price with a fudged tolerance. Seeded and deterministic.
- Determinism: identical results for a given seed regardless of thread count. There is a
  test for this; do not weaken it.
- Degenerate-case tests are first-class: vol-of-vol → 0 must recover Black–Scholes,
  barrier → ∞ must recover the vanilla price. Add more of these where you spot the
  opportunity.
- Golden-file regression tests on surrogate outputs; update them only with an explicit
  note explaining why the numbers moved.

## Benchmarks

- Record a baseline **before** any optimisation. Every performance claim is a measured
  delta, never an estimate.
- `benchmarks/` regenerates every figure and table with one command, from a clean
  checkout.
- Report p50/p99 for service latency, paths/sec and thread scaling for the engine.

## Style

- C++20. Warnings as errors. Sanitizers in CI. No allocation on the hot path;
  structure-of-arrays layouts. `clang-tidy` clean.
- Python: type hints, `ruff` clean, config-driven experiments (YAML), fixed seeds.
- Prefer clarity over cleverness anywhere I'll have to explain the code out loud.
- No new third-party dependency without asking first.

## Non-goals

Do not add: GPU inference serving, Kubernetes, a multi-product pricing library in Phase 1,
market-data ingestion (all inputs are generated — this is deliberate), or any alpha or
trading claim. Over-infrastructure reads as résumé-driven engineering and is worse than
nothing.
