// analytic.h : closed-form prices. The oracle every later step is measured against.
//
// THE SIGNATURES BELOW ARE A STRAWMAN, NOT A DECISION.
//
// They exist so the library target, the test linkage and the golden-file fixture
// are wired and provably working before any mathematics is written. The shape of
// this API — cost-of-carry b versus separate r/q, flat arguments versus a
// parameter struct, where rebates live — is yours to settle. Changing it costs
// one line in cpp/tests/analytic_test.cpp.
//
// The definitions in analytic.cpp throw. Replace them.

#pragma once

namespace deep_pricer {

// Generalised Black-Scholes, cost-of-carry form: b = r for a non-dividend stock,
// b = r - q with a continuous dividend yield, b = 0 for a future. This is the
// convention Haug's barrier formulas use, which is why it is the strawman.
double black_scholes_call(double S, double K, double T, double r, double b, double sigma);
double black_scholes_put(double S, double K, double T, double r, double b, double sigma);

// Up-and-out call, Reiner-Rubinstein. H is the barrier, rebate paid on knock-out.
double up_and_out_call(double S, double K, double H, double rebate, double T, double r,
                       double b, double sigma);

// Up-and-in call. Worth writing alongside the out variant even though Step 2 does
// not ask for it: in-out parity (in + out == vanilla, zero rebate) is the single
// strongest check on the sign conventions, and it reuses the same terms.
double up_and_in_call(double S, double K, double H, double rebate, double T, double r,
                      double b, double sigma);

}  // namespace deep_pricer
