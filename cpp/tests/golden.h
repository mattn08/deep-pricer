// golden.h : loads reference-value tables for the analytic tests.
//
// A golden file is CSV with a header row. Blank lines and lines beginning with
// '#' are ignored, so a file can carry provenance notes at the top. Every row
// must name its source, because a value nobody can trace is not ground truth.

#pragma once

#include <string>
#include <vector>

namespace deep_pricer::testing {

struct GoldenRow {
  // Where this number came from: book, edition, table and row, or the script and
  // revision that generated it. Printed on failure.
  std::string source;
  // Short human-readable case name, used to name the test.
  std::string label;

  double S = 0.0;
  double K = 0.0;
  double H = 0.0;
  double rebate = 0.0;
  double T = 0.0;
  double r = 0.0;
  double b = 0.0;
  double sigma = 0.0;

  // The reference price, and the tolerance appropriate to *this source*. A value
  // transcribed from a table printed to 4 decimals does not deserve the same
  // tolerance as one generated at 50 digits, and the file should say so per row
  // rather than the test assuming one global epsilon.
  double expected = 0.0;
  double rel_tolerance = 0.0;
  double abs_tolerance = 0.0;
};

// Returns an empty vector if the file does not exist, so a fixture can skip
// loudly rather than fail while the table is still to be generated.
// Throws std::runtime_error on a malformed file — a silently half-read table of
// reference values would be worse than no table.
std::vector<GoldenRow> load_golden(const std::string& path);

}  // namespace deep_pricer::testing
