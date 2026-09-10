//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalCopperProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", ThermalCopperProperties);

InputParameters
ThermalCopperProperties::validParams()
{
  InputParameters params = ThermalSolidProperties::validParams();

  MooseEnum rrr("RRR_50 RRR_100 RRR_150 RRR_300 RRR_500", "RRR_100");
  params.addParam<MooseEnum>(
      "rrr", rrr, "Residual resistivity ratio for thermal conductivity (affects low-T behavior)");
  params.addRangeCheckedParam<Real>(
      "density", 8940.0, "density > 0.0", "Density of OFHC copper [kg/m³]");
  params.addClassDescription("Thermal properties of oxygen-free high-conductivity (OFHC) copper "
                             "from NIST Cryogenic Materials Database. Valid range: 4-300 K.");
  return params;
}

ThermalCopperProperties::ThermalCopperProperties(const InputParameters & parameters)
  : ThermalSolidProperties(parameters),
    _rrr(getParam<MooseEnum>("rrr").getEnum<RRRValue>()),
    _rho_const(getParam<Real>("density")),
    // Thermal conductivity coefficients for RRR = 50
    _k50_a(1.8743),
    _k50_b(-0.41538),
    _k50_c(-0.6018),
    _k50_d(0.13294),
    _k50_e(0.26426),
    _k50_f(-0.0219),
    _k50_g(-0.051276),
    _k50_h(0.0014871),
    _k50_i(0.003723),
    // Thermal conductivity coefficients for RRR = 100
    _k100_a(2.2154),
    _k100_b(-0.47461),
    _k100_c(-0.88068),
    _k100_d(0.13871),
    _k100_e(0.29505),
    _k100_f(-0.02043),
    _k100_g(-0.04831),
    _k100_h(0.001281),
    _k100_i(0.003207),
    // Thermal conductivity coefficients for RRR = 150
    _k150_a(2.3797),
    _k150_b(-0.4918),
    _k150_c(-0.98615),
    _k150_d(0.13942),
    _k150_e(0.30475),
    _k150_f(-0.019713),
    _k150_g(-0.046897),
    _k150_h(0.0011969),
    _k150_i(0.0029988),
    // Thermal conductivity coefficients for RRR = 300
    _k300_a(1.357),
    _k300_b(0.3981),
    _k300_c(2.669),
    _k300_d(-0.1346),
    _k300_e(-0.6683),
    _k300_f(0.01342),
    _k300_g(0.05773),
    _k300_h(0.0002147),
    _k300_i(0.0),
    // Thermal conductivity coefficients for RRR = 500
    _k500_a(2.8075),
    _k500_b(-0.54074),
    _k500_c(-1.2777),
    _k500_d(0.15362),
    _k500_e(0.36444),
    _k500_f(-0.02105),
    _k500_g(-0.051727),
    _k500_h(0.0012226),
    _k500_i(0.0030964),
    // Specific heat coefficients (RRR-independent)
    // log10(cp) = a + b*log10(T) + c*log10(T)^2 + ... + h*log10(T)^7
    _cp_a(-1.91844),
    _cp_b(-0.15973),
    _cp_c(8.61013),
    _cp_d(-18.996),
    _cp_e(21.9661),
    _cp_f(-12.7328),
    _cp_g(3.54322),
    _cp_h(-0.3797)
{
}

Real
ThermalCopperProperties::k_from_T(const Real & T) const
{
  if ((T < 4.0) || (T > 300.0))
    flagInvalidSolution("Thermal conductivity evaluated outside valid range [4, 300] K");

  Real k, dk_dT;

  switch (_rrr)
  {
    case RRR_50:
      computeThermalConductivity(
          T, _k50_a, _k50_b, _k50_c, _k50_d, _k50_e, _k50_f, _k50_g, _k50_h, _k50_i, k, dk_dT);
      break;
    case RRR_100:
      computeThermalConductivity(T,
                                 _k100_a,
                                 _k100_b,
                                 _k100_c,
                                 _k100_d,
                                 _k100_e,
                                 _k100_f,
                                 _k100_g,
                                 _k100_h,
                                 _k100_i,
                                 k,
                                 dk_dT);
      break;
    case RRR_150:
      computeThermalConductivity(T,
                                 _k150_a,
                                 _k150_b,
                                 _k150_c,
                                 _k150_d,
                                 _k150_e,
                                 _k150_f,
                                 _k150_g,
                                 _k150_h,
                                 _k150_i,
                                 k,
                                 dk_dT);
      break;
    case RRR_300:
      computeThermalConductivity(T,
                                 _k300_a,
                                 _k300_b,
                                 _k300_c,
                                 _k300_d,
                                 _k300_e,
                                 _k300_f,
                                 _k300_g,
                                 _k300_h,
                                 _k300_i,
                                 k,
                                 dk_dT);
      break;
    case RRR_500:
      computeThermalConductivity(T,
                                 _k500_a,
                                 _k500_b,
                                 _k500_c,
                                 _k500_d,
                                 _k500_e,
                                 _k500_f,
                                 _k500_g,
                                 _k500_h,
                                 _k500_i,
                                 k,
                                 dk_dT);
      break;
    default:
      mooseError("Unhandled RRRValue enum!");
  }

  return k;
}

void
ThermalCopperProperties::k_from_T(const Real & T, Real & k, Real & dk_dT) const
{
  if ((T < 4.0) || (T > 300.0))
    flagInvalidSolution("Thermal conductivity evaluated outside valid range [4, 300] K");

  switch (_rrr)
  {
    case RRR_50:
      computeThermalConductivity(
          T, _k50_a, _k50_b, _k50_c, _k50_d, _k50_e, _k50_f, _k50_g, _k50_h, _k50_i, k, dk_dT);
      break;
    case RRR_100:
      computeThermalConductivity(T,
                                 _k100_a,
                                 _k100_b,
                                 _k100_c,
                                 _k100_d,
                                 _k100_e,
                                 _k100_f,
                                 _k100_g,
                                 _k100_h,
                                 _k100_i,
                                 k,
                                 dk_dT);
      break;
    case RRR_150:
      computeThermalConductivity(T,
                                 _k150_a,
                                 _k150_b,
                                 _k150_c,
                                 _k150_d,
                                 _k150_e,
                                 _k150_f,
                                 _k150_g,
                                 _k150_h,
                                 _k150_i,
                                 k,
                                 dk_dT);
      break;
    case RRR_300:
      computeThermalConductivity(T,
                                 _k300_a,
                                 _k300_b,
                                 _k300_c,
                                 _k300_d,
                                 _k300_e,
                                 _k300_f,
                                 _k300_g,
                                 _k300_h,
                                 _k300_i,
                                 k,
                                 dk_dT);
      break;
    case RRR_500:
      computeThermalConductivity(T,
                                 _k500_a,
                                 _k500_b,
                                 _k500_c,
                                 _k500_d,
                                 _k500_e,
                                 _k500_f,
                                 _k500_g,
                                 _k500_h,
                                 _k500_i,
                                 k,
                                 dk_dT);
      break;
    default:
      mooseError("Unhandled RRRValue enum!");
  }
}

void
ThermalCopperProperties::computeThermalConductivity(const Real & T,
                                                    const Real & a,
                                                    const Real & b,
                                                    const Real & c,
                                                    const Real & d,
                                                    const Real & e,
                                                    const Real & f,
                                                    const Real & g,
                                                    const Real & h,
                                                    const Real & i,
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
ThermalCopperProperties::cp_from_T(const Real & T) const
{
  if ((T < 4.0) || (T > 300.0))
    flagInvalidSolution("Specific heat evaluated outside valid range [4, 300] K");

  // NIST correlation: log10(cp) = sum of polynomial in log10(T)
  const Real log10_T = std::log10(T);
  const Real log10_T2 = Utility::pow<2>(log10_T);
  const Real log10_T3 = Utility::pow<3>(log10_T);
  const Real log10_T4 = Utility::pow<4>(log10_T);
  const Real log10_T5 = Utility::pow<5>(log10_T);
  const Real log10_T6 = Utility::pow<6>(log10_T);
  const Real log10_T7 = Utility::pow<7>(log10_T);

  const Real log10_cp = _cp_a + _cp_b * log10_T + _cp_c * log10_T2 + _cp_d * log10_T3 +
                        _cp_e * log10_T4 + _cp_f * log10_T5 + _cp_g * log10_T6 + _cp_h * log10_T7;

  return std::pow(10.0, log10_cp);
}

void
ThermalCopperProperties::cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const
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

  const Real log10_cp = _cp_a + _cp_b * log10_T + _cp_c * log10_T2 + _cp_d * log10_T3 +
                        _cp_e * log10_T4 + _cp_f * log10_T5 + _cp_g * log10_T6 + _cp_h * log10_T7;

  cp = std::pow(10.0, log10_cp);

  // Derivative: dcp/dT = cp * ln(10) * d(log10_cp)/d(log10_T) * d(log10_T)/dT
  // d(log10_cp)/d(log10_T) = b + 2c*log10_T + 3d*log10_T^2 + ...
  // d(log10_T)/dT = 1/(T*ln(10))
  const Real dlog10cp_dlog10T = _cp_b + 2.0 * _cp_c * log10_T + 3.0 * _cp_d * log10_T2 +
                                4.0 * _cp_e * log10_T3 + 5.0 * _cp_f * log10_T4 +
                                6.0 * _cp_g * log10_T5 + 7.0 * _cp_h * log10_T6;

  dcp_dT = cp * dlog10cp_dlog10T / T;
}

Real
ThermalCopperProperties::rho_from_T(const Real & /* T */) const
{
  return _rho_const;
}

void
ThermalCopperProperties::rho_from_T(const Real & T, Real & rho, Real & drho_dT) const
{
  rho = rho_from_T(T);
  drho_dT = 0.0;
}
