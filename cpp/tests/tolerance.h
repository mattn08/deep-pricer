// tolerance.h : one comparison used by every numerical test.
//
// EXPECT_NEAR takes an absolute tolerance only, which is the wrong instrument
// when prices in one table span several orders of magnitude: a tolerance loose
// enough for a price of 50 is meaningless for one of 1e-6. This combines a
// relative tolerance with an absolute floor, so a deep out-of-the-money price
// near zero is judged on the floor and everything else on relative error.
//
// On failure it prints the absolute and relative error and the source of the
// reference value, because "expected 4.2 got 4.1" does not tell you which table
// row to go and check.

#pragma once

#include <gtest/gtest.h>

#include <cmath>
#include <string>

namespace deep_pricer::testing {

inline ::testing::AssertionResult IsClose(double actual, double expected, double rel_tolerance,
                                          double abs_tolerance, const std::string& source = {}) {
  const double abs_error = std::abs(actual - expected);
  const double scale = std::abs(expected);
  const double rel_error = scale > 0.0 ? abs_error / scale : 0.0;

  if (abs_error <= abs_tolerance || rel_error <= rel_tolerance) {
    return ::testing::AssertionSuccess();
  }

  ::testing::AssertionResult failure = ::testing::AssertionFailure();
  failure << "\n  actual   = " << actual << "\n  expected = " << expected
          << "\n  abs error = " << abs_error << " (tolerance " << abs_tolerance << ")"
          << "\n  rel error = " << rel_error << " (tolerance " << rel_tolerance << ")";
  if (!source.empty()) {
    failure << "\n  reference = " << source;
  }
  return failure;
}

}  // namespace deep_pricer::testing
