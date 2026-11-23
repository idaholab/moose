//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElectromagneticOFHCCopperProperties.h"
#include "libmesh/utility.h"
#include <cmath>

registerMooseObject("SolidPropertiesApp", ElectromagneticOFHCCopperProperties);

InputParameters
ElectromagneticOFHCCopperProperties::validParams()
{
  InputParameters params = ElectromagneticSolidProperties::validParams();

  params.addRangeCheckedParam<Real>(
      "rrr",
      100.0,
      "rrr > 0.0",
      "Residual resistivity ratio (RRR) for electrical resistivity. "
      "RRR = rho(273K) / rho(4K) characterizes sample purity. "
      "Higher RRR indicates higher purity and lower residual resistivity.");

  params.addClassDescription(
      "Electromagnetic properties of oxygen-free high-conductivity (OFHC) copper "
      "from NIST Monograph 177, Chapter 8. Valid range: 2-900 K.");

  return params;
}

ElectromagneticOFHCCopperProperties::ElectromagneticOFHCCopperProperties(
    const InputParameters & parameters)
  : ElectromagneticSolidProperties(parameters),
    _T_min(2.0),
    _T_max(900.0),
    _rrr(getParam<Real>("rrr")),
    // NIST zero-field copper electrical-resistivity correlation
    // rho_0 = 1.553e-8 / RRR [Ohm*m]
    _rho_0(1.553e-8 / _rrr),
    // NIST copper coefficients (SI units, already in Ohm*m)
    _P1(1.171e-17),
    _P2(4.49),
    _P3(3.841e10),
    _P4(1.14),
    _P5(50.0),
    _P6(6.428),
    _P7(0.4531),
    _rho_c(0.0),
    // Magnetic permeability: mu_0 = 4*pi * 10^-7 H/m
    _mu_const(1.25663706212e-6)
{
}

Real
ElectromagneticOFHCCopperProperties::electrical_resistivity_from_T(const Real & T) const
{
  Real rho, drho_dT;
  electrical_resistivity_from_T(T, rho, drho_dT);
  return rho;
}

void
ElectromagneticOFHCCopperProperties::electrical_resistivity_from_T(const Real & T,
                                                                   Real & rho,
                                                                   Real & drho_dT) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution("Electrical resistivity evaluated outside valid range [2, 900] K");

  // Three-component model: rho(T) = rho_0 + rho_i(T) + rho_i0(T)
  Real rho_i, rho_i0, drho_i_dT, drho_i0_dT;
  computeElectricalResistivity(T, rho_i, rho_i0, drho_i_dT, drho_i0_dT);

  rho = _rho_0 + rho_i + rho_i0;
  drho_dT = drho_i_dT + drho_i0_dT; // rho_0 is constant, so d(rho_0)/dT = 0
}

Real
ElectromagneticOFHCCopperProperties::electrical_conductivity_from_T(const Real & T) const
{
  Real sigma, dsigma_dT;
  electrical_conductivity_from_T(T, sigma, dsigma_dT);
  return sigma;
}

void
ElectromagneticOFHCCopperProperties::electrical_conductivity_from_T(const Real & T,
                                                                    Real & sigma,
                                                                    Real & dsigma_dT) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution("Electrical conductivity evaluated outside valid range [2, 900] K");

  // sigma = 1/rho
  Real rho, drho_dT;
  electrical_resistivity_from_T(T, rho, drho_dT);

  sigma = 1.0 / rho;

  // d(1/rho)/dT = -1/rho^2 * drho/dT (chain rule)
  dsigma_dT = -drho_dT / (rho * rho);
}

Real
ElectromagneticOFHCCopperProperties::magnetic_permeability_from_T(const Real & /* T */) const
{
  return _mu_const;
}

void
ElectromagneticOFHCCopperProperties::magnetic_permeability_from_T(const Real & T,
                                                                  Real & mu,
                                                                  Real & dmu_dT) const
{
  mu = magnetic_permeability_from_T(T);
  dmu_dT = 0.0; // Constant property
}

void
ElectromagneticOFHCCopperProperties::computeElectricalResistivity(
    Real T, Real & rho_i, Real & rho_i0, Real & drho_i_dT, Real & drho_i0_dT) const
{
  // NIST zero-field copper electrical-resistivity correlation
  // All values in SI units (Ohm*m) - NO unit conversion needed
  //
  // rho_i(T) = (P1 * T^P2) / (1 + P1 * P3 * T^(P2-P4) * exp(-(P5/T)^P6)) + rho_c
  //
  // rho_i0(T) = P7 * rho_i(T) * rho_0 / (rho_i(T) + rho_0)

  // ===== Compute rho_i(T) =====
  // Break down the complex expression for clarity and derivative calculation
  const Real T_P2 = std::pow(T, _P2);                // T^P2
  const Real T_P2_minus_P4 = std::pow(T, _P2 - _P4); // T^(P2-P4)
  const Real P5_over_T_P6 = std::pow(_P5 / T, _P6);  // (P5/T)^P6
  const Real exp_term = std::exp(-P5_over_T_P6);     // exp(-(P5/T)^P6)

  const Real numer = _P1 * T_P2;                                 // Numerator
  const Real denom = 1.0 + _P1 * _P3 * T_P2_minus_P4 * exp_term; // Denominator

  rho_i = numer / denom + _rho_c; // [Ohm*m]

  // ===== Compute drho_i/dT using quotient rule =====
  // d/dT[f/g] = (f' * g - f * g') / g^2

  // f = P1 * T^P2,  f' = P1 * P2 * T^(P2-1)
  const Real df_dT = _P1 * _P2 * std::pow(T, _P2 - 1.0);

  // g = 1 + P1 * P3 * T^(P2-P4) * exp(-(P5/T)^P6)
  // g' = P1 * P3 * d/dT[T^(P2-P4) * exp(-(P5/T)^P6)]
  // Using product rule: d/dT[u * v] = u' * v + u * v'

  const Real dT_pow_dT = (_P2 - _P4) * std::pow(T, _P2 - _P4 - 1.0); // d/dT[T^(P2-P4)]

  // d/dT[exp(-(P5/T)^P6)] = exp(...) * d/dT[-(P5/T)^P6]
  //                       = exp(...) * (-P6) * (P5/T)^(P6-1) * d/dT[P5/T]
  //                       = exp(...) * (-P6) * (P5/T)^(P6-1) * (-P5/T^2)
  //                       = exp(...) * P6 * (P5/T)^(P6-1) * P5 / T^2
  const Real dexp_dT = exp_term * _P6 * std::pow(_P5 / T, _P6 - 1.0) * _P5 / (T * T);

  const Real dg_dT = _P1 * _P3 * (dT_pow_dT * exp_term + T_P2_minus_P4 * dexp_dT);

  drho_i_dT = (df_dT * denom - numer * dg_dT) / (denom * denom);

  // ===== Compute rho_i0(T) =====
  // rho_i0 = P7 * rho_i * rho_0 / (rho_i + rho_0)
  const Real sum_rho = rho_i + _rho_0;
  rho_i0 = _P7 * rho_i * _rho_0 / sum_rho; // [Ohm*m]

  // ===== Compute drho_i0/dT using quotient rule =====
  // rho_i0 = (P7 * rho_0 * rho_i) / (rho_i + rho_0)
  // Let A = P7 * rho_0 * rho_i,  B = rho_i + rho_0
  // drho_i0/dT = (dA/dT * B - A * dB/dT) / B^2

  const Real A = _P7 * _rho_0 * rho_i;
  const Real dA_dT = _P7 * _rho_0 * drho_i_dT; // rho_0 is constant
  const Real dB_dT = drho_i_dT;                // rho_0 is constant

  drho_i0_dT = (dA_dT * sum_rho - A * dB_dT) / (sum_rho * sum_rho);
}
