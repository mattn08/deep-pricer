# Deep Pricer

A neural-network surrogate for a Monte Carlo exotic-derivatives pricer.

Risk that takes seconds of simulation, served in microseconds — with the engineering path
from research code to production service done properly.

> **Status:** in development. Milestones are tagged; see [Project status](#project-status).

<!-- Fill once Step 11 is done and you have real figures:
**TL;DR** — prices and Greeks for an up-and-out barrier option under Heston, `[X]`× faster
than converged Monte Carlo at `[X]` bp mean absolute error inside the training domain, with
automatic fallback to the MC engine outside it.
-->

---

## Why this project

Exotic derivatives risk is expensive to compute. A barrier option under stochastic
volatility has no closed form, so a desk revalues by Monte Carlo — and a full risk grid
across spot and vol ladders means thousands of revaluations. That cost is what makes
intraday risk on exotic books hard.

A trained surrogate can approximate the pricing function itself, returning prices *and*
Greeks in a single forward pass. The interesting problem is not the neural network; it is
everything around it — generating training data that is cheap but unbiased, computing
differentials that are correct, knowing where the surrogate can and cannot be trusted, and
building a path from research code to a production service that a risk function would
accept.

This repository is that path, built end to end.

## What it does

- Prices an **up-and-out European call** by Monte Carlo under Black–Scholes and Heston,
  in multithreaded C++, with pathwise Greeks.
- Trains a **differential-ML surrogate** (Huge & Savine) on single-path samples and their
  pathwise adjoint sensitivities.
- Serves the surrogate over HTTP with **out-of-domain detection** and automatic fallback
  to the Monte Carlo engine.
- Regenerates every figure and number in this README from a clean checkout with one
  command.

Phase 2 adds a **worst-of autocallable** as a payoff plugin, to demonstrate that the
architecture is product-agnostic rather than fitted to one payoff.

## Design decisions

The decisions below are the substance of the project. Each is a choice with a defensible
alternative.

### Barrier option first, autocallable second

The up-and-out call has a closed-form price under Black–Scholes (Reiner–Rubinstein). That
gives every layer of the stack — MC engine, pathwise Greeks, surrogate — a ground truth to
be measured against before moving to dynamics where no closed form exists. It also has
genuinely hard Greeks: delta and gamma explode near the barrier as expiry approaches,
which is exactly the stress case where a surrogate either works or visibly fails.

The autocallable is the opposite trade-off — closer to real equities-desk work, but no
closed form and many more parameters. It arrives in Phase 2, once the validation
scaffolding exists to support it.

### Ground truth at every step

The governing rule of the build: no step is finished until its output is checked against
something already trusted. Analytic formulae where they exist; converged MC with its
standard error where they don't; an independent NumPy implementation as a second opinion;
degenerate cases (vol-of-vol → 0 must recover Black–Scholes) as cheap structural checks.

Tolerances are never loosened to make a discrepancy disappear.

### Brownian-bridge barrier handling, doing two jobs

Discretely-monitored paths systematically miss barrier crossings between time steps, which
biases the price upward relative to the continuously-monitored contract. The Brownian
bridge gives the probability of crossing between two observed points, which corrects that
bias.

The same construction solves a second problem. The knock-out indicator is not
differentiable, so pathwise sensitivities of a hard indicator are useless. Replacing the
indicator with the bridge *survival probability* — a conditional expectation — makes the
payoff smooth in the parameters and reduces variance at the same time. One piece of
mathematics, two problems.

### Differential machine learning, not price regression

The naive approach trains on converged MC prices: expensive labels, few of them. The
differential approach samples the *parameter space* and takes one path per point, recording
the discounted payoff and its pathwise differentials. Labels are extremely noisy but
extremely cheap, and the differentials carry far more information per sample than the value
alone. The regression averages the noise away.

An ablation at matched data budgets — differential loss versus value-only loss — quantifies
what the differential term actually buys. <!-- link the figure here once Step 10 lands -->

### An out-of-domain guard

Neural networks extrapolate silently and confidently outside their training domain. A
pricing service that does that is worse than no service at all.

The exported model carries its training-domain bounds as metadata. Requests outside those
bounds are flagged and routed to the Monte Carlo engine instead — slower, but correct. The
service reports which path served each request.

## Architecture

```
deep-pricer/
  cpp/          # MC engine, payoffs, pathwise adjoints, pybind11 module
  python/       # dataset generation, training, ONNX export
  service/      # FastAPI app, ONNX runtime, Dockerfile
  benchmarks/   # every figure in this README regenerates from here
  agents/       # LLM harnesses: validation sweeps, experiment runs, reports
  experiments/  # agent-written sweep summaries (committed)
  configs/      # YAML experiment configs
  docs/         # spec.md, build-order.md, design notes
```

<!-- Add a short data-flow diagram here once the pipeline is end-to-end. -->

## Results

<!-- Placeholder. Fill from benchmarks/ output as each step lands — never by hand, and
     never without the reference and the domain stated alongside. -->

| Metric | Result | Reference |
|---|---|---|
| Price accuracy (in-domain) | `[X]` | converged MC, `[X]` paths, ±`[X]` SE |
| Delta / vega accuracy | `[X]` | bump-and-revalue MC |
| Surrogate inference latency | p50 `[X]` / p99 `[X]` | — |
| 10k-instrument risk grid | `[X]` vs `[X]` | full MC revaluation |
| MC engine throughput | `[X]` paths/sec (1 thread), `[X]`× scaling to `[X]` cores | — |
| Differential vs value-only loss | `[X]` | matched data budget |

**Where the surrogate is unreliable:** `[X]` <!-- the near-barrier, short-expiry region is
the expected answer — state it plainly and show the heatmap. -->

## Running it

```bash
# Build and test the C++ engine
cmake -B build -DCMAKE_BUILD_TYPE=Release && cmake --build build && ctest --test-dir build

# Generate a training dataset
python -m python.dataset --config configs/barrier_heston.yaml

# Train the surrogate
python -m python.train --config configs/surrogate_base.yaml

# Serve it
docker compose up

# Regenerate every figure in this README
make benchmarks
```

<!-- Keep these honest as the interfaces settle — a README whose commands don't run is
     worse than one with fewer commands. -->

## AI-assisted workflow

The numerical core — the Monte Carlo engine, payoff sensitivities, pathwise adjoints and
the differential loss — is hand-written. Agents own the harness around it, and everything
they produce is committed rather than discarded:

- a **validation agent** that sweeps the parameter space, finds regions where error exceeds
  statistical tolerance, bisects to isolate them, and writes up a diagnosis with the
  reproducing parameters;
- an **experiment agent** that proposes and runs hyperparameter sweeps within a budget,
  reads the tracking output, and commits a summary per sweep to `experiments/`;
- a **reporting agent** that turns benchmark artefacts into a model-validation-style
  document covering accuracy, domain of validity and failure modes.

The split is deliberate. The mathematics is where the thinking is, so it is mine; the
sweeps, diagnostics and reporting are tedious and mechanical, which is exactly what an
agent should absorb.

## References

- Huge & Savine, *Differential Machine Learning* (2020) — the training approach used here.
- Reiner & Rubinstein, *Breaking Down the Barriers* (1991) — closed-form barrier prices.
- Glasserman, *Monte Carlo Methods in Financial Engineering* — discretisation, variance
  reduction, pathwise sensitivities.
- Andersen & Piterbarg, *Interest Rate Modeling*, vol. 1 — full-truncation Euler for Heston.
- Fries & Joshi, *Conditional analytic Monte-Carlo pricing scheme of auto-callable products*
  (2008) — smoothing trigger discontinuities; the basis for the Phase 2 payoff.

## Project status

- [ ] **1** — Repo skeleton and CI
- [ ] **2** — Closed-form Black–Scholes and barrier pricers
- [ ] **3** — Monte Carlo engine, validated against closed form
- [ ] **4** — Path generation, discrete barrier monitoring, Brownian-bridge correction
- [ ] **5** — Sobol QMC, multithreading, benchmarks
- [ ] **6** — Heston dynamics
- [ ] **7** — pybind11 bindings and independent NumPy reference
- [ ] **8** — Pathwise adjoint Greeks
- [ ] **9** — Differential training dataset
- [ ] **10** — Surrogate training and ablation
- [ ] **11** — ONNX export, FastAPI service, out-of-domain guard
- [ ] **12** — Agent harnesses, documentation, reproducibility pass
- [ ] **Phase 2** — Worst-of autocallable payoff plugin

## Licence and disclaimer

MIT — see [LICENSE](LICENSE).

A personal research and educational project, built independently on personal equipment. It
is not investment advice, contains no proprietary code, data or models from any employer,
and is not affiliated with or endorsed by any organisation. No warranty of any kind; the
numbers here are demonstrations, not production risk.
