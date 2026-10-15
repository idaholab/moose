//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MechanicalCryogenicJK2LBProperties.h"
#include "libmesh/utility.h"

registerMooseObject("SolidPropertiesApp", MechanicalCryogenicJK2LBProperties);

InputParameters
MechanicalCryogenicJK2LBProperties::validParams()
{
  InputParameters params = MechanicalSolidProperties::validParams();

  params.addClassDescription(
      "Mechanical properties of JK2LB austenitic stainless steel for cryogenic applications. "
      "Young's modulus from PPPL-4687 report, Poisson's ratio and thermal expansion from Lu et "
      "al. (2008). Valid range: 4-300 K.");

  return params;
}

MechanicalCryogenicJK2LBProperties::MechanicalCryogenicJK2LBProperties(
    const InputParameters & parameters)
  : MechanicalSolidProperties(parameters),
    _T_min_E(4.0),
    _T_max_E(293.0),
    _T_min_alpha(10.0),
    _T_max_alpha(300.0),
    // Young's modulus coefficients from PPPL-4687 linear correlation
    // E(4K) = 200 GPa, E(293K) = 192 GPa
    // E(T) = E_0 + E_1 * T [Pa]
    _E_0(2.001107266435986e+11),
    _E_1(-2.768166089965398e+07),
    // Poisson's ratio from Lu et al. (2008)
    _nu_const(2.680000000000000e-01),
    // Thermal expansion strain coefficients from quintic fit to Lu et al. (2008) data
    // strain(T) = c0 + c1*T + c2*T^2 + c3*T^3 + c4*T^4 + c5*T^5
    // alpha(T) = d(strain)/dT = c1 + 2*c2*T + 3*c3*T^2 + 4*c4*T^3 + 5*c5*T^4
    // Fitted via scipy.optimize.curve_fit to digitized data
    // R^2 = 0.9998376, RMSE = 9.41e-4, Max Error = 2.25e-3
    _alpha_c0(-2.118054889305034e-01),
    _alpha_c1(-5.080573166927160e-05),
    _alpha_c2(2.732213037411437e-07),
    _alpha_c3(3.014378817610530e-08),
    _alpha_c4(-1.413458116508355e-10),
    _alpha_c5(2.235355010061840e-13)
{
}

Real
MechanicalCryogenicJK2LBProperties::E_from_T(const Real & T) const
{
  Real E, dE_dT;
  E_from_T(T, E, dE_dT);
  return E;
}

void
MechanicalCryogenicJK2LBProperties::E_from_T(const Real & T, Real & E, Real & dE_dT) const
{
  if ((T < _T_min_E) || (T > _T_max_E))
    flagInvalidSolution("Young's modulus evaluated outside valid range [" +
                        std::to_string(_T_min_E) + ", " + std::to_string(_T_max_E) + "] K");

  // Linear correlation: E(T) = E_0 + E_1 * T
  E = _E_0 + _E_1 * T;

  // Derivative: dE/dT = E_1 (constant)
  dE_dT = _E_1;
}

Real
MechanicalCryogenicJK2LBProperties::nu_from_T(const Real & T) const
{
  Real nu, dnu_dT;
  nu_from_T(T, nu, dnu_dT);
  return nu;
}

void
MechanicalCryogenicJK2LBProperties::nu_from_T(const Real & T, Real & nu, Real & dnu_dT) const
{
  if ((T < _T_min_E) || (T > _T_max_E))
    flagInvalidSolution("Poisson's ratio evaluated outside valid range [" +
                        std::to_string(_T_min_E) + ", " + std::to_string(_T_max_E) + "] K");

  // Constant from Lu et al. (2008)
  nu = _nu_const;
  dnu_dT = 0.0;
}

Real
MechanicalCryogenicJK2LBProperties::alpha_from_T(const Real & T) const
{
  Real alpha, dalpha_dT;
  alpha_from_T(T, alpha, dalpha_dT);
  return alpha;
}

void
MechanicalCryogenicJK2LBProperties::alpha_from_T(const Real & T,
                                                 Real & alpha,
                                                 Real & dalpha_dT) const
{
  if ((T < _T_min_alpha) || (T > _T_max_alpha))
    flagInvalidSolution("Thermal expansion coefficient evaluated outside valid range [" +
                        std::to_string(_T_min_alpha) + ", " + std::to_string(_T_max_alpha) + "] K");

  // Thermal expansion coefficient: alpha(T) = d(strain)/dT
  // where strain(T) = c0 + c1*T + c2*T^2 + c3*T^3 + c4*T^4 + c5*T^5
  // Therefore: alpha(T) = c1 + 2*c2*T + 3*c3*T^2 + 4*c4*T^3 + 5*c5*T^4
  const Real T2 = Utility::pow<2>(T);
  const Real T3 = Utility::pow<3>(T);
  const Real T4 = Utility::pow<4>(T);

  alpha = _alpha_c1 + 2.0 * _alpha_c2 * T + 3.0 * _alpha_c3 * T2 + 4.0 * _alpha_c4 * T3 +
          5.0 * _alpha_c5 * T4;

  // Derivative: dalpha/dT = 2*c2 + 6*c3*T + 12*c4*T^2 + 20*c5*T^3
  dalpha_dT = 2.0 * _alpha_c2 + 6.0 * _alpha_c3 * T + 12.0 * _alpha_c4 * T2 + 20.0 * _alpha_c5 * T3;
}
