//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalCryogenicJK2LBProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalCryogenicJK2LBProperties);

InputParameters
ThermalCryogenicJK2LBProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();

  params.addRangeCheckedParam<Real>(
      "density", 7830.0, "density > 0.0", "Density of JK2LB steel [kg/m^3]");
  params.addRangeCheckedParam<unsigned int>(
      "cp_integral_n_intervals",
      1000,
      "cp_integral_n_intervals > 0",
      "Number of intervals for composite trapezoid integration of specific heat cp");

  params.addClassDescription(
      "Thermal properties of JK2LB austenitic stainless steel for cryogenic applications. "
      "Data from digitized experimental measurements fitted with log-polynomial correlations "
      "(functional form matches NIST patterns for cryogenic metals). Valid range: 2-300 K.");

  return params;
}

ThermalCryogenicJK2LBProperties::ThermalCryogenicJK2LBProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _T_min(2.0),
    _T_max(300.0),
    _rho_const(getParam<Real>("density")),
    _cp_integral_n_intervals(getParam<unsigned int>("cp_integral_n_intervals")),
    // Thermal conductivity coefficients from log-polynomial fit
    // log10(k) = c0 + c1*log10(T) + c2*log10(T)^2 + c3*log10(T)^3 + c4*log10(T)^4
    // Fitted via scipy.optimize.curve_fit to digitized data from:
    // https://www.sciencedirect.com/science/article/pii/S0011227508001835
    // R^2 = 0.999832, RMSE = 0.0416 W/(m*K), Max Error = 0.08 W/(m*K)
    _k_c0(-2.107316501340309e-01),
    _k_c1(-1.550912560071352e+00),
    _k_c2(2.855060632258989e+00),
    _k_c3(-1.321082156932475e+00),
    _k_c4(2.030855338304893e-01),
    // Specific heat coefficients from log-polynomial fit
    // log10(cp) = c0 + c1*log10(T) + c2*log10(T)^2 + ... + c7*log10(T)^7
    // Fitted via scipy.optimize.curve_fit to digitized data from same source
    // R^2 = 0.999898, RMSE = 1.59 J/(kg*K), Max Error = 5.6 J/(kg*K)
    _cp_c0(-8.424917216275501e+00),
    _cp_c1(3.582054612333810e+01),
    _cp_c2(-5.461579969849875e+01),
    _cp_c3(3.270614351111416e+01),
    _cp_c4(4.335375881686904e-01),
    _cp_c5(-8.544246444111970e+00),
    _cp_c6(3.366314156009067e+00),
    _cp_c7(-4.137625699962628e-01)
{
}

Real
ThermalCryogenicJK2LBProperties::k_from_T(const Real & T) const
{
  Real k, dk_dT;
  k_from_T(T, k, dk_dT);
  return k;
}

void
ThermalCryogenicJK2LBProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution("Thermal conductivity evaluated outside valid range [" +
                        std::to_string(_T_min) + ", " + std::to_string(_T_max) + "] K");

  // Log-polynomial correlation: log10(k) = sum(c_i * log10(T)^i) for i=0..4
  const Real log10_T = std::log10(T);
  const Real log10_T2 = Utility::pow<2>(log10_T);
  const Real log10_T3 = Utility::pow<3>(log10_T);
  const Real log10_T4 = Utility::pow<4>(log10_T);

  const Real log10_k =
      _k_c0 + _k_c1 * log10_T + _k_c2 * log10_T2 + _k_c3 * log10_T3 + _k_c4 * log10_T4;

  // k = 10^(log10_k)
  k = std::pow(10.0, log10_k);

  // Derivative: dk/dT = k * d(log10_k)/d(log10_T) * d(log10_T)/dT
  // d(log10_k)/d(log10_T) = c1 + 2*c2*log10_T + 3*c3*log10_T^2 + 4*c4*log10_T^3
  // d(log10_T)/dT = 1/(T*ln(10))
  const Real dlog10k_dlog10T =
      _k_c1 + 2.0 * _k_c2 * log10_T + 3.0 * _k_c3 * log10_T2 + 4.0 * _k_c4 * log10_T3;

  dk_dT = k * dlog10k_dlog10T / T;
}

Real
ThermalCryogenicJK2LBProperties::cp_from_T(const Real & T) const
{
  Real cp, dcp_dT;
  cp_from_T(T, cp, dcp_dT);
  return cp;
}

void
ThermalCryogenicJK2LBProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution("Specific heat evaluated outside valid range [" + std::to_string(_T_min) +
                        ", " + std::to_string(_T_max) + "] K");

  // Log-polynomial correlation: log10(cp) = sum(c_i * log10(T)^i) for i=0..7
  // Same functional form as NIST uses for cryogenic copper and 304 stainless steel
  const Real log10_T = std::log10(T);
  const Real log10_T2 = Utility::pow<2>(log10_T);
  const Real log10_T3 = Utility::pow<3>(log10_T);
  const Real log10_T4 = Utility::pow<4>(log10_T);
  const Real log10_T5 = Utility::pow<5>(log10_T);
  const Real log10_T6 = Utility::pow<6>(log10_T);
  const Real log10_T7 = Utility::pow<7>(log10_T);

  const Real log10_cp = _cp_c0 + _cp_c1 * log10_T + _cp_c2 * log10_T2 + _cp_c3 * log10_T3 +
                        _cp_c4 * log10_T4 + _cp_c5 * log10_T5 + _cp_c6 * log10_T6 +
                        _cp_c7 * log10_T7;

  // cp = 10^(log10_cp)
  cp = std::pow(10.0, log10_cp);

  // Derivative: dcp/dT = cp * d(log10_cp)/d(log10_T) * d(log10_T)/dT
  const Real dlog10cp_dlog10T = _cp_c1 + 2.0 * _cp_c2 * log10_T + 3.0 * _cp_c3 * log10_T2 +
                                4.0 * _cp_c4 * log10_T3 + 5.0 * _cp_c5 * log10_T4 +
                                6.0 * _cp_c6 * log10_T5 + 7.0 * _cp_c7 * log10_T6;

  dcp_dT = cp * dlog10cp_dlog10T / T;
}

Real
ThermalCryogenicJK2LBProperties::rho_from_T(const Real & /* T */) const
{
  return _rho_const;
}

void
ThermalCryogenicJK2LBProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}

Real
ThermalCryogenicJK2LBProperties::cp_integral(const Real & T) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution("cp_integral evaluated outside valid range [" + std::to_string(_T_min) +
                        ", " + std::to_string(_T_max) + "] K");

  // Numerical integration using composite trapezoid rule
  // Integral of cp(T) from T_min to T
  const Real dT = (T - _T_min) / static_cast<Real>(_cp_integral_n_intervals);
  Real sum = 0.0;

  for (unsigned int i = 0; i <= _cp_integral_n_intervals; ++i)
  {
    const Real T_i = _T_min + static_cast<Real>(i) * dT;
    const Real weight = (i == 0 || i == _cp_integral_n_intervals) ? 0.5 : 1.0;
    sum += weight * cp_from_T(T_i);
  }

  return sum * dT;
}
