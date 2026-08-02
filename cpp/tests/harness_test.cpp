// harness_test.cpp : proves the GoogleTest harness itself works.
//
// There is no engine to test yet, so these assert nothing about pricing. They
// exist so that the wiring — discovery, exact and tolerance-based assertions,
// and the value-parameterised pattern the parameter-grid tests will use — is
// known to work before the first real test is written. Replace them as the
// engine lands; they are scaffolding, not coverage.

#include <gtest/gtest.h>

#include <cmath>

TEST(Harness, ExactAssertionsWork) {
  EXPECT_EQ(2 + 2, 4);
  EXPECT_TRUE(true);
}

// The two floating-point styles the numerical tests will need: EXPECT_DOUBLE_EQ
// for closed-form results (equality to within 4 ULP) and EXPECT_NEAR for a
// tolerance computed at run time, which is how a Monte Carlo price gets checked
// against its own standard error.
TEST(Harness, FloatingPointAssertionsWork) {
  // sqrt is correctly rounded, so this is exact on any IEEE-754 target.
  EXPECT_DOUBLE_EQ(std::sqrt(4.0), 2.0);

  const double estimate = 10.02;
  const double reference = 10.00;
  const double standard_error = 0.01;
  EXPECT_NEAR(estimate, reference, 3.0 * standard_error);
}

// Value-parameterised tests report each case as a separately named test, which
// is how a grid of pricing parameters should be swept.
class HarnessGrid : public ::testing::TestWithParam<int> {};

TEST_P(HarnessGrid, ReceivesItsParameter) {
  EXPECT_GT(GetParam(), 0);
}

INSTANTIATE_TEST_SUITE_P(SmallGrid, HarnessGrid, ::testing::Values(1, 2, 3));
