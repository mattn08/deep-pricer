#include "golden.h"

#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <map>
#include <stdexcept>
#include <utility>

namespace deep_pricer::testing {
namespace {

// Minimal RFC 4180 field splitting: fields may be double-quoted, a quoted field
// may contain the delimiter, and "" inside a quoted field is a literal quote.
// Needed because a citation is naturally comma-rich ("Haug, 2007, Table 4-13")
// and so are generated labels.
std::vector<std::string> split(const std::string& line, char delimiter) {
  std::vector<std::string> fields;
  std::string field;
  bool quoted = false;

  for (std::size_t i = 0; i < line.size(); ++i) {
    const char c = line[i];
    if (quoted) {
      if (c == '"') {
        if (i + 1 < line.size() && line[i + 1] == '"') {
          field.push_back('"');
          ++i;
        } else {
          quoted = false;
        }
      } else {
        field.push_back(c);
      }
    } else if (c == '"') {
      quoted = true;
    } else if (c == delimiter) {
      fields.push_back(field);
      field.clear();
    } else {
      field.push_back(c);
    }
  }
  fields.push_back(field);

  if (quoted) {
    throw std::runtime_error("golden file: unterminated quoted field");
  }
  return fields;
}

bool is_skippable(const std::string& line) {
  for (const char c : line) {
    if (std::isspace(static_cast<unsigned char>(c)) != 0) {
      continue;
    }
    return c == '#';
  }
  return true;  // blank
}

double to_double(const std::string& text, const std::string& column, std::size_t line_number) {
  try {
    return std::stod(text);
  } catch (const std::exception&) {
    throw std::runtime_error("golden file line " + std::to_string(line_number) + ": column '" +
                             column + "' is not a number: '" + text + "'");
  }
}

}  // namespace

std::vector<GoldenRow> load_golden(const std::string& path) {
  if (!std::filesystem::exists(path)) {
    return {};
  }

  std::ifstream file(path);
  if (!file) {
    throw std::runtime_error("cannot open golden file: " + path);
  }

  std::vector<std::string> header;
  std::vector<GoldenRow> rows;
  std::string line;
  std::size_t line_number = 0;

  while (std::getline(file, line)) {
    ++line_number;
    if (!line.empty() && line.back() == '\r') {
      line.pop_back();
    }
    if (is_skippable(line)) {
      continue;
    }

    std::vector<std::string> fields = split(line, ',');
    if (header.empty()) {
      header = std::move(fields);
      continue;
    }

    if (fields.size() != header.size()) {
      throw std::runtime_error("golden file line " + std::to_string(line_number) + ": expected " +
                               std::to_string(header.size()) + " fields, found " +
                               std::to_string(fields.size()));
    }

    std::map<std::string, std::string> cells;
    for (std::size_t i = 0; i < header.size(); ++i) {
      cells[header[i]] = fields[i];
    }

    const auto number = [&](const std::string& column) {
      const auto it = cells.find(column);
      if (it == cells.end()) {
        throw std::runtime_error("golden file: missing required column '" + column + "'");
      }
      return to_double(it->second, column, line_number);
    };
    const auto text = [&](const std::string& column) -> std::string {
      const auto it = cells.find(column);
      return it == cells.end() ? std::string{} : it->second;
    };

    GoldenRow row;
    row.source = text("source");
    row.label = text("label");
    row.S = number("S");
    row.K = number("K");
    row.T = number("T");
    row.r = number("r");
    row.b = number("b");
    row.sigma = number("sigma");
    row.expected = number("expected");
    row.rel_tolerance = number("rel_tolerance");
    row.abs_tolerance = number("abs_tolerance");
    // Barrier columns are optional so one loader serves vanilla and barrier tables.
    row.H = cells.contains("H") ? number("H") : 0.0;
    row.rebate = cells.contains("rebate") ? number("rebate") : 0.0;

    if (row.source.empty()) {
      throw std::runtime_error("golden file line " + std::to_string(line_number) +
                               ": every row must name its source");
    }

    rows.push_back(std::move(row));
  }

  return rows;
}

}  // namespace deep_pricer::testing
