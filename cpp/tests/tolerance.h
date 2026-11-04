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
#include <limits>
#include <string>

namespace deep_pricer::testing {

inline ::testing::AssertionResult IsClose(double actual, double expected, double rel_tolerance,
                                          double abs_tolerance, const std::string& source = {}) {
  const double abs_error = std::abs(actual - expected);
  const double scale = std::abs(expected);

  // The relative test is a product rather than a ratio, so a zero reference, which
  // has no magnitude to be relative to, is judged on the absolute floor alone. Any
  // comparison with NaN is false, so a NaN result can never pass.
  if (abs_error <= abs_tolerance || abs_error <= rel_tolerance * scale) {
    return ::testing::AssertionSuccess();
  }

  // Only needed for the message. Infinite against a zero reference, which is the
  // honest answer to "how far off is this, relative to zero?".
  const double rel_error =
      scale > 0.0 ? abs_error / scale : std::numeric_limits<double>::infinity();

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
