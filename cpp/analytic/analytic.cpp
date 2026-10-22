// analytic.cpp : deliberately unimplemented.
//
// Every definition here throws. The mathematics is yours to write; this file
// exists only so the library links and the harness around it can be proven to
// work. Parameter names are omitted so the unused-parameter warning does not
// fire while the bodies are stubs — restore them as you implement each one.

#include "analytic.h"

#include <cmath>
#include <stdexcept>
#include <string>

namespace deep_pricer {
namespace {

[[noreturn]] void not_implemented(const char* what) {
  throw std::logic_error(std::string("not implemented: ") + what);
}

inline constexpr double kInvSqrt2 = 0.707106781186547524400844362104849039;

}  // namespace


// Standard normal cumulative distribution function (CDF)
double norm_cdf(const double& x) {
	return 0.5 * std::erfc(-x * kInvSqrt2);
}

// This calculates d_j, for j in {1 ,2}. This term appears in the closed
// form solution for the European call or put price. The drift is the cost of
// carry b, not the rate r: the forward is S*e^{bT}, and r only discounts.
double d_j(const int& j, const double& S, const double& K, const double& b,
	const double& v, const double& T) {
	return (log(S / K) + (b + (pow(-1.0, j - 1)) * 0.5 * v * v) * T) / (v * (pow(T, 0.5)));

}


// Generalised Black-Scholes. e^{(b-r)T} is 1 when b = r (no dividends), so this
// reduces to the textbook formula; b = r - q gives Merton, b = 0 gives Black-76.
double black_scholes_call(double S, double K, double T, double r, double b, double sigma) {
	return S * exp((b - r) * T) * norm_cdf(d_j(1, S, K, b, sigma, T)) - K * exp(-r * T) *
		norm_cdf(d_j(2, S, K, b, sigma, T));
}

double black_scholes_put(double S, double K, double T, double r, double b, double sigma) {
	return -S * exp((b - r) * T) * norm_cdf(-d_j(1, S, K, b, sigma, T)) + K * exp(-r * T) *
		norm_cdf(-d_j(2, S, K, b, sigma, T));
}

double up_and_out_call(double, double, double, double, double, double, double, double) {
  not_implemented("up_and_out_call");
}

double up_and_in_call(double, double, double, double, double, double, double, double) {
  not_implemented("up_and_in_call");
}



}  // namespace deep_pricer
