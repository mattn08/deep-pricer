// analytic.cpp : deliberately unimplemented.
//
// Every definition here throws. The mathematics is yours to write; this file
// exists only so the library links and the harness around it can be proven to
// work. Parameter names are omitted so the unused-parameter warning does not
// fire while the bodies are stubs — restore them as you implement each one.

#include "analytic.h"

#include <stdexcept>
#include <string>

namespace deep_pricer {
namespace {

[[noreturn]] void not_implemented(const char* what) {
  throw std::logic_error(std::string("not implemented: ") + what);
}

}  // namespace

double black_scholes_call(double, double, double, double, double, double) {
  not_implemented("black_scholes_call");
}

double black_scholes_put(double, double, double, double, double, double) {
  not_implemented("black_scholes_put");
}

double up_and_out_call(double, double, double, double, double, double, double, double) {
  not_implemented("up_and_out_call");
}

double up_and_in_call(double, double, double, double, double, double, double, double) {
  not_implemented("up_and_in_call");
}

}  // namespace deep_pricer
