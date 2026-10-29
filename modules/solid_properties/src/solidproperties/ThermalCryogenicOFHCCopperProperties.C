//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalCryogenicOFHCCopperProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalCryogenicOFHCCopperProperties);

InputParameters
ThermalCryogenicOFHCCopperProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();

  MooseEnum rrr("RRR_50 RRR_100 RRR_150 RRR_300 RRR_500", "RRR_100");
  params.addParam<MooseEnum>(
      "rrr", rrr, "Residual resistivity ratio for thermal conductivity (affects low-T behavior)");
  params.addRangeCheckedParam<Real>(
      "density", 8940.0, "density > 0.0", "Density of OFHC copper [kg/m^3]");
  params.addRangeCheckedParam<unsigned int>(
      "cp_integral_n_intervals",
      1000,
      "cp_integral_n_intervals > 0",
      "Number of intervals for composite trapezoid integration of NIST specific heat cp");
  params.addClassDescription("Thermal properties of oxygen-free high-conductivity (OFHC) copper "
                             "from NIST Cryogenic Materials Database. Valid range: 4-300 K.");
  return params;
}

ThermalCryogenicOFHCCopperProperties::ThermalCryogenicOFHCCopperProperties(
    const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _rrr(getParam<MooseEnum>("rrr").getEnum<RRRValue>()),
    _rrr_index(static_cast<unsigned int>(_rrr)),
    _rho_const(getParam<Real>("density")),
    _cp_integral_n_intervals(getParam<unsigned int>("cp_integral_n_intervals")),
    // Thermal conductivity coefficients for this instance's RRR
    _k_a(_ka_values[_rrr_index]),
    _k_b(_kb_values[_rrr_index]),
    _k_c(_kc_values[_rrr_index]),
    _k_d(_kd_values[_rrr_index]),
    _k_e(_ke_values[_rrr_index]),
    _k_f(_kf_values[_rrr_index]),
    _k_g(_kg_values[_rrr_index]),
    _k_h(_kh_values[_rrr_index]),
    _k_i(_ki_values[_rrr_index])
{
}

Real
ThermalCryogenicOFHCCopperProperties::k_from_T(const Real & T) const
{
  Real k, dk_dT;
  k_from_T(T, k, dk_dT);
  return k;
}

void
ThermalCryogenicOFHCCopperProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  if ((T < 4.0) || (T > 300.0))
    flagInvalidSolution("Thermal conductivity evaluated outside valid range [4, 300] K");

  computeThermalConductivity(T, _k_a, _k_b, _k_c, _k_d, _k_e, _k_f, _k_g, _k_h, _k_i, k, dk_dT);
}

void
ThermalCryogenicOFHCCopperProperties::computeThermalConductivity(Real T,
                                                                 Real a,
                                                                 Real b,
                                                                 Real c,
                                                                 Real d,
                                                                 Real e,
                                                                 Real f,
                                                                 Real g,
                                                                 Real h,
                                                                 Real i,
                                                                 Real & k,
                                                                 Real & dk_dT) const
{
  // NIST correlation: log10(k) = numerator / denominator
  // where numerator = a + c*T^0.5 + e*T + g*T^1.5 + i*T^2
  //       denominator = 1 + b*T^0.5 + d*T + f*T^1.5 + h*T^2

  const Real sqrt_T = std::sqrt(T);
  const Real T_1_5 = T * sqrt_T;
  const Real T_2 = T * T;

  // Compute numerator and its derivative
  const Real numer = a + c * sqrt_T + e * T + g * T_1_5 + i * T_2;
  const Real dnumer_dT = 0.5 * c / sqrt_T + e + 1.5 * g * sqrt_T + 2.0 * i * T;

  // Compute denominator and its derivative
  const Real denom = 1.0 + b * sqrt_T + d * T + f * T_1_5 + h * T_2;
  const Real ddenom_dT = 0.5 * b / sqrt_T + d + 1.5 * f * sqrt_T + 2.0 * h * T;

  // log10(k) = numer / denom
  const Real log10_k = numer / denom;

  // k = 10^(numer/denom)
  k = std::pow(10.0, log10_k);

  // dk/dT using chain rule:
  // d(10^x)/dT = 10^x * ln(10) * dx/dT
  // dx/dT = d(numer/denom)/dT = (dnumer_dT * denom - numer * ddenom_dT) / denom^2
  const Real dlog10_k_dT = (dnumer_dT * denom - numer * ddenom_dT) / (denom * denom);
  dk_dT = k * std::log(10.0) * dlog10_k_dT;
}

Real
ThermalCryogenicOFHCCopperProperties::cp_from_T(const Real & T) const
{
  Real cp, dcp_dT;
  cp_from_T(T, cp, dcp_dT);
  return cp;
}

void
ThermalCryogenicOFHCCopperProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
{
  if ((T < 4.0) || (T > 300.0))
    flagInvalidSolution("Specific heat evaluated outside valid range [4, 300] K");

  const Real log10_T = std::log10(T);
  const Real log10_T2 = Utility::pow<2>(log10_T);
  const Real log10_T3 = Utility::pow<3>(log10_T);
  const Real log10_T4 = Utility::pow<4>(log10_T);
  const Real log10_T5 = Utility::pow<5>(log10_T);
  const Real log10_T6 = Utility::pow<6>(log10_T);
  const Real log10_T7 = Utility::pow<7>(log10_T);

  const Real log10_cp = _cp_coeff[0] + _cp_coeff[1] * log10_T + _cp_coeff[2] * log10_T2 +
                        _cp_coeff[3] * log10_T3 + _cp_coeff[4] * log10_T4 +
                        _cp_coeff[5] * log10_T5 + _cp_coeff[6] * log10_T6 + _cp_coeff[7] * log10_T7;

  cp = std::pow(10.0, log10_cp);

  // Derivative: dcp/dT = cp * ln(10) * d(log10_cp)/d(log10_T) * d(log10_T)/dT
  // d(log10_cp)/d(log10_T) = b + 2c*log10_T + 3d*log10_T^2 + ...
  // d(log10_T)/dT = 1/(T*ln(10))
  const Real dlog10cp_dlog10T = _cp_coeff[1] + 2.0 * _cp_coeff[2] * log10_T +
                                3.0 * _cp_coeff[3] * log10_T2 + 4.0 * _cp_coeff[4] * log10_T3 +
                                5.0 * _cp_coeff[5] * log10_T4 + 6.0 * _cp_coeff[6] * log10_T5 +
                                7.0 * _cp_coeff[7] * log10_T6;

  dcp_dT = cp * dlog10cp_dlog10T / T;
}

Real
ThermalCryogenicOFHCCopperProperties::rho_from_T(const Real & /* T */) const
{
  return _rho_const;
}

void
ThermalCryogenicOFHCCopperProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}

Real
ThermalCryogenicOFHCCopperProperties::cp_integral(const Real & T) const
{
  // Numerical integration of cp(T) from 4 K to T using trapezoidal rule.
  // The complex NIST log-polynomial correlation for cp cannot be integrated analytically.

  const Real T_min = 4.0; // Lower bound of NIST correlation validity

  if (T < T_min)
    mooseError("cp_integral called with T = ",
               T,
               " K < T_min = ",
               T_min,
               " K. ",
               "Temperature must be within the valid range [4, 300] K.");

  const Real dT = (T - T_min) / static_cast<Real>(_cp_integral_n_intervals);
  Real integral = 0.0;

  // Trapezoidal integration rule
  for (unsigned int i = 0; i <= _cp_integral_n_intervals; ++i)
  {
    const Real T_i = T_min + static_cast<Real>(i) * dT;
    const Real cp_i = cp_from_T(T_i);

    if (i == 0 || i == _cp_integral_n_intervals)
      integral += 0.5 * cp_i; // Endpoints weighted by 0.5
    else
      integral += cp_i; // Interior points weighted by 1.0
  }

  return integral * dT;
}
