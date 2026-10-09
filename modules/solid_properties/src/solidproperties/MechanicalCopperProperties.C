//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicalCopperProperties.h"
#include "libmesh/utility.h"
#include <cmath>

registerMooseObject("SolidPropertiesApp", MechanicalCopperProperties);

InputParameters
MechanicalCopperProperties::validParams()
{
  InputParameters params = MechanicalSolidProperties::validParams();
  params.addClassDescription(
      "Mechanical properties of oxygen-free high-conductivity (OFHC) copper.");
  return params;
}

MechanicalCopperProperties::MechanicalCopperProperties(const InputParameters & parameters)
  : MechanicalSolidProperties(parameters),
    _T_min(4.0),
    _T_max(300.0),
    // Young's modulus: E(T) = 1e9 × (137 - 1.27e-4 × T²) [Pa]
    _E_c0(137.0),
    _E_c2(-1.27e-4),
    // Poisson's ratio: ν(T) = 0.339 + 7.03e-8 × T²
    _nu_c0(0.339),
    _nu_c2(7.03e-8),
    // Thermal expansion: log₁₀(α [10⁻⁶/K]) = Σ cᵢ × [log₁₀(T)]ⁱ
    _alpha_c0(-17.9081289),
    _alpha_c1(67.131914),
    _alpha_c2(-118.809316),
    _alpha_c3(109.9845997),
    _alpha_c4(-53.8696089),
    _alpha_c5(13.30247491),
    _alpha_c6(-1.30843441)
{
}

Real
MechanicalCopperProperties::E_from_T(const Real & T) const
{
  Real E, dE_dT;
  E_from_T(T, E, dE_dT);
  return E;
}

void
MechanicalCopperProperties::E_from_T(const Real & T, Real & E, Real & dE_dT) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution("Young's modulus evaluated outside valid range [4, 300] K");

  // E(T) = 1e9 × (137 - 1.27e-4 × T²) [Pa]
  const Real T2 = Utility::pow<2>(T);
  E = 1.0e9 * (_E_c0 + _E_c2 * T2);

  // dE/dT = 1e9 × (-1.27e-4 × 2T) = -2.54e5 × T [Pa/K]
  dE_dT = 1.0e9 * _E_c2 * 2.0 * T;
}

Real
MechanicalCopperProperties::nu_from_T(const Real & T) const
{
  Real nu, dnu_dT;
  nu_from_T(T, nu, dnu_dT);
  return nu;
}

void
MechanicalCopperProperties::nu_from_T(const Real & T, Real & nu, Real & dnu_dT) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution("Poisson's ratio evaluated outside valid range [4, 300] K");

  // ν(T) = 0.339 + 7.03e-8 × T²
  const Real T2 = Utility::pow<2>(T);
  nu = _nu_c0 + _nu_c2 * T2;

  // dν/dT = 7.03e-8 × 2T = 1.406e-7 × T
  dnu_dT = _nu_c2 * 2.0 * T;
}

Real
MechanicalCopperProperties::alpha_from_T(const Real & T) const
{
  Real alpha, dalpha_dT;
  alpha_from_T(T, alpha, dalpha_dT);
  return alpha;
}

void
MechanicalCopperProperties::alpha_from_T(const Real & T, Real & alpha, Real & dalpha_dT) const
{
  if ((T < _T_min) || (T > _T_max))
    flagInvalidSolution(
        "Coefficient of thermal expansion evaluated outside valid range [4, 300] K");

  // log₁₀(α [10⁻⁶/K]) = Σ cᵢ × [log₁₀(T)]ⁱ for i=0..6
  const Real coeffs[7] = {
      _alpha_c0, _alpha_c1, _alpha_c2, _alpha_c3, _alpha_c4, _alpha_c5, _alpha_c6};

  const Real log10_T = std::log10(T);

  // Compute polynomial sum and its derivative with respect to log₁₀(T)
  Real poly_sum = 0.0;
  Real poly_deriv_sum = 0.0;
  Real log10_T_power = 1.0;
  Real log10_T_power_prev = 1.0; // [log₁₀(T)]^(i-1)

  for (unsigned int i = 0; i < 7; ++i)
  {
    // Add term i: cᵢ × [log₁₀(T)]^i
    poly_sum += coeffs[i] * log10_T_power;

    // Add derivative term i: i × cᵢ × [log₁₀(T)]^(i-1)
    if (i > 0)
      poly_deriv_sum += static_cast<Real>(i) * coeffs[i] * log10_T_power_prev;

    // Update powers for next iteration
    log10_T_power_prev = log10_T_power;
    log10_T_power *= log10_T;
  }

  // α(T) = 10^(poly_sum - 6) [1/K]
  // The correlation gives log₁₀(α × 10⁶) = poly_sum, so α = 10^(poly_sum) / 10⁶ = 10^(poly_sum-6)
  alpha = std::pow(10.0, poly_sum - 6.0);

  // Derivative calculation:
  // α = 10^(P-6)
  // dα/dT = 10^(P-6) × ln(10) × dP/dT
  // dP/dT = (dP/d(log₁₀(T))) × (d(log₁₀(T))/dT)
  //       = poly_deriv_sum × 1/(T × ln(10))
  // Therefore: dα/dT = 10^(P-6) × ln(10) × poly_deriv_sum / (T × ln(10))
  //                  = α × poly_deriv_sum / T
  dalpha_dT = alpha * poly_deriv_sum / T;
}
