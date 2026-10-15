//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MechanicalSolidProperties.h"

/**
 * Mechanical properties of JK2LB austenitic stainless steel for cryogenic applications.
 *
 * Valid range: 4-300 K
 *
 * Young's modulus: Linear correlation from PPPL-4687 report
 *   E(T) = E_0 + E_1 * T
 *   Data: E(4K) = 200 GPa, E(293K) = 192 GPa
 *
 * Poisson's ratio: Constant from Lu et al. (2008)
 *   nu = 0.268
 *
 * Thermal expansion coefficient: Quintic polynomial from strain data
 *   alpha(T) = d(strain)/dT where strain fitted from Lu et al. (2008)
 *   strain(T) = c0 + c1*T + c2*T^2 + c3*T^3 + c4*T^4 + c5*T^5
 *   alpha(T) = c1 + 2*c2*T + 3*c3*T^2 + 4*c4*T^3 + 5*c5*T^4
 *   Valid range: 10-300 K (thermal expansion data range)
 */
class MechanicalCryogenicJK2LBProperties : public MechanicalSolidProperties
{
public:
  static InputParameters validParams();

  MechanicalCryogenicJK2LBProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  // Young's modulus
  virtual Real E_from_T(const Real & T) const override;
  virtual void E_from_T(const Real & T, Real & E, Real & dE_dT) const override;

  // Poisson's ratio
  virtual Real nu_from_T(const Real & T) const override;
  virtual void nu_from_T(const Real & T, Real & nu, Real & dnu_dT) const override;

  // Thermal expansion coefficient
  virtual Real alpha_from_T(const Real & T) const override;
  virtual void alpha_from_T(const Real & T, Real & alpha, Real & dalpha_dT) const override;

#pragma GCC diagnostic pop

protected:
  /// Valid temperature range for Young's modulus and Poisson's ratio
  const Real _T_min_E;
  const Real _T_max_E;

  /// Valid temperature range for thermal expansion coefficient
  const Real _T_min_alpha;
  const Real _T_max_alpha;

  /// Young's modulus coefficients: E(T) = E_0 + E_1 * T [Pa]
  /// Linear correlation from PPPL-4687: E(4K)=200 GPa, E(293K)=192 GPa
  const Real _E_0;
  const Real _E_1;

  /// Poisson's ratio: constant from Lu et al. (2008)
  const Real _nu_const;

  /// Thermal expansion strain coefficients: strain(T) = c0 + c1*T + ... + c5*T^5
  /// Fitted via scipy.optimize.curve_fit to digitized data from Lu et al. (2008)
  /// https://www.sciencedirect.com/science/article/pii/S0011227508001835
  /// R^2 = 0.9998376, RMSE = 9.41e-4, Max Error = 2.25e-3
  /// Thermal expansion coefficient: alpha(T) = d(strain)/dT
  const Real _alpha_c0;
  const Real _alpha_c1;
  const Real _alpha_c2;
  const Real _alpha_c3;
  const Real _alpha_c4;
  const Real _alpha_c5;
};
