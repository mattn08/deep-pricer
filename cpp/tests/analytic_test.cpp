// analytic_test.cpp : the harness for Step 2.
//
// Two kinds of test live here.
//
// 1. Golden-file tests, which compare against reference values from a named
//    source. Adding a case is a line of CSV, not a line of C++. They skip while
//    a table is absent, so the harness can be merged before the numbers exist.
//
// 2. Invariant tests, which need no reference values at all — put-call parity,
//    in-out parity, the barrier limit, arbitrage bounds. These catch the sign
//    and assembly errors that transcribed values often miss.
//
// Every invariant test is prefixed DISABLED_ because the functions it calls
// throw today. Drop the prefix as you implement each one; that is the intended
// order of work, and the list below doubles as the checklist for Step 2.

#include "../analytic/analytic.h"
#include "golden.h"
#include "tolerance.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

using deep_pricer::testing::GoldenRow;
using deep_pricer::testing::IsClose;

std::string data_path(const std::string& name) {
  return std::string(DEEP_PRICER_TEST_DATA_DIR) + "/" + name;
}

// Runs a golden table through a pricing function. Skips loudly when the table is
// not there yet rather than passing silently, which would be indistinguishable
// from a table of zero rows that all agree.
template <typename Price>
void check_table(const std::string& file, Price price) {
  const std::string path = data_path(file);
  const std::vector<GoldenRow> rows = deep_pricer::testing::load_golden(path);
  if (rows.empty()) {
    GTEST_SKIP() << "no reference table at " << path
                 << " — generate it with python/tools/gen_reference.py and commit it";
  }
  for (const GoldenRow& row : rows) {
    SCOPED_TRACE(row.label.empty() ? row.source : row.label);
    EXPECT_TRUE(IsClose(price(row), row.expected, row.rel_tolerance, row.abs_tolerance, row.source));
  }
}

TEST(AnalyticGolden, BlackScholesCall) {
  check_table("black_scholes_call.csv", [](const GoldenRow& row) {
    return deep_pricer::black_scholes_call(row.S, row.K, row.T, row.r, row.b, row.sigma);
  });
}

TEST(AnalyticGolden, UpAndOutCall) {
  check_table("up_and_out_call.csv", [](const GoldenRow& row) {
    return deep_pricer::up_and_out_call(row.S, row.K, row.H, row.rebate, row.T, row.r, row.b,
                                        row.sigma);
  });
}

// --- Invariants. Remove DISABLED_ as each becomes implementable. ------------

// C - P == S*e^{(b-r)T} - K*e^{-rT}. Catches discounting and carry errors that a
// single transcribed price will not.
TEST(AnalyticInvariant, DISABLED_PutCallParity) {
  FAIL() << "implement black_scholes_call/put, then assert parity across a seeded grid";
}

// Vol to zero leaves a deterministic discounted payoff. One of the degenerate
// cases CLAUDE.md asks to be treated as first-class.
TEST(AnalyticInvariant, DISABLED_ZeroVolatilityIsDeterministic) {
  FAIL() << "implement the sigma -> 0 branch and assert the discounted forward payoff";
}

// Barrier far above spot cannot knock out, so the price must collapse onto the
// vanilla. This is the strongest single check on the barrier term assembly.
TEST(AnalyticInvariant, DISABLED_DistantBarrierRecoversVanilla) {
  FAIL() << "assert up_and_out_call with H -> large equals black_scholes_call";
}

// in + out == vanilla, with zero rebate. The best available check on the eta/phi
// sign conventions, and the reason up_and_in_call is worth writing now.
TEST(AnalyticInvariant, DISABLED_InOutParity) {
  FAIL() << "assert up_and_in_call + up_and_out_call equals black_scholes_call";
}

// Spot already at or above the barrier means the option is dead on arrival and
// worth exactly the rebate.
TEST(AnalyticInvariant, DISABLED_SpotAtBarrierIsRebate) {
  FAIL() << "assert up_and_out_call with S >= H equals the rebate";
}

// An up-and-out call whose barrier sits at or below the strike can only pay out
// where it has already knocked out, so it is worthless. A small positive number
// here is the classic Reiner-Rubinstein assembly bug.
TEST(AnalyticInvariant, DISABLED_BarrierBelowStrikeIsWorthless) {
  FAIL() << "assert up_and_out_call with H <= K is zero (plus any rebate)";
}

// max(0, S*e^{(b-r)T} - K*e^{-rT}) <= C <= S*e^{(b-r)T}, and the knock-out can
// never be worth more than the vanilla.
TEST(AnalyticInvariant, DISABLED_ArbitrageBounds) {
  FAIL() << "assert the no-arbitrage bounds and up_and_out_call <= black_scholes_call";
}

}  // namespace
