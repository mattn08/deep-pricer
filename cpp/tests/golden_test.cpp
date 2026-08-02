// golden_test.cpp : tests for the loader itself.
//
// The loader stands between a reference table and every numerical assertion made
// against it. A parser that silently mis-reads a row would turn ground truth into
// a plausible-looking lie, so it gets tested like anything else.

#include "golden.h"
#include "tolerance.h"

#include <gtest/gtest.h>

#include <string>
#include <vector>

namespace {

using deep_pricer::testing::GoldenRow;
using deep_pricer::testing::IsClose;
using deep_pricer::testing::load_golden;

std::string data_path(const std::string& name) {
  return std::string(DEEP_PRICER_TEST_DATA_DIR) + "/" + name;
}

TEST(GoldenLoader, ReadsRowsSkippingCommentsAndBlanks) {
  const std::vector<GoldenRow> rows = load_golden(data_path("loader_fixture.csv"));
  ASSERT_EQ(rows.size(), 2U);

  EXPECT_EQ(rows[0].label, "plain");
  EXPECT_DOUBLE_EQ(rows[0].S, 1.0);
  EXPECT_DOUBLE_EQ(rows[0].K, 2.0);
  EXPECT_DOUBLE_EQ(rows[0].H, 3.0);
  EXPECT_DOUBLE_EQ(rows[0].rebate, 4.0);
  EXPECT_DOUBLE_EQ(rows[0].T, 5.0);
  EXPECT_DOUBLE_EQ(rows[0].r, 6.0);
  EXPECT_DOUBLE_EQ(rows[0].b, 7.0);
  EXPECT_DOUBLE_EQ(rows[0].sigma, 8.0);
  EXPECT_DOUBLE_EQ(rows[0].expected, 9.0);
  EXPECT_DOUBLE_EQ(rows[0].rel_tolerance, 1e-12);
  EXPECT_DOUBLE_EQ(rows[0].abs_tolerance, 1e-14);
}

// The case that motivated quoted-field support: a citation contains commas, and
// naive splitting would shift every subsequent column by three places.
TEST(GoldenLoader, QuotedFieldKeepsItsCommas) {
  const std::vector<GoldenRow> rows = load_golden(data_path("loader_fixture.csv"));
  ASSERT_EQ(rows.size(), 2U);
  EXPECT_EQ(rows[0].source, "Haug, 2007, Table 4-13");
}

TEST(GoldenLoader, EscapedQuoteBecomesLiteralQuote) {
  const std::vector<GoldenRow> rows = load_golden(data_path("loader_fixture.csv"));
  ASSERT_EQ(rows.size(), 2U);
  EXPECT_EQ(rows[1].source, "a \"quoted\" source");
  EXPECT_DOUBLE_EQ(rows[1].S, 10.0);
}

// A missing table is not an error: the fixture skips, so the harness can be
// merged before any reference values exist.
TEST(GoldenLoader, MissingFileYieldsNoRows) {
  EXPECT_TRUE(load_golden(data_path("no_such_table.csv")).empty());
}

TEST(GoldenLoader, MalformedFileThrows) {
  EXPECT_THROW(static_cast<void>(load_golden(data_path("loader_malformed.csv"))),
               std::runtime_error);
}

// --- the tolerance helper --------------------------------------------------

TEST(Tolerance, RelativeToleranceScalesWithMagnitude) {
  // 1e-9 relative error: passes at 1e-8 relative, fails at 1e-10, on a large value
  // where a fixed absolute tolerance would be the wrong instrument.
  EXPECT_TRUE(IsClose(1.000000001e6, 1.0e6, 1e-8, 0.0));
  EXPECT_FALSE(IsClose(1.000000001e6, 1.0e6, 1e-10, 0.0));
}

TEST(Tolerance, AbsoluteFloorCarriesNearZero) {
  // Relative error is 100% here, but both numbers are inside the floor, which is
  // the deep out-of-the-money case.
  EXPECT_TRUE(IsClose(2e-16, 1e-16, 1e-12, 1e-14));
  EXPECT_FALSE(IsClose(2e-16, 1e-16, 1e-12, 1e-17));
}

TEST(Tolerance, FailureMessageNamesTheSource) {
  const ::testing::AssertionResult result = IsClose(1.0, 2.0, 1e-12, 1e-14, "Haug Table 4-13");
  EXPECT_FALSE(static_cast<bool>(result));
  EXPECT_NE(std::string(result.message()).find("Haug Table 4-13"), std::string::npos);
}

}  // namespace
