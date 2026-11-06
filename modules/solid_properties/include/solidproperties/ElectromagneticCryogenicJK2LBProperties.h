//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElectromagneticSolidProperties.h"

/**
 * Electromagnetic properties of JK2LB austenitic stainless steel for cryogenic applications.
 *
 * Valid range: 2-300 K
 *
 * Electrical resistivity: Quintic polynomial from Lu et al. (2009)
 *   rho(T) = c0 + c1*T + c2*T^2 + c3*T^3 + c4*T^4 + c5*T^5 [Ohm·m]
 *   Data: Digitized electrical resistivity measurements
 *   Fit: R^2 = 0.9987, RMSE = 8.03e-10 Ohm·m
 *   Captures antiferromagnetic transition signature at Neel temperature (240 K)
 *
 * Electrical conductivity: Reciprocal of resistivity
 *   sigma(T) = 1/rho(T) [S/m]
 *
 * Magnetic permeability: Constant (austenitic steel is non-magnetic)
 *   mu = mu_0 = 4*pi*10^-7 H/m
 */
class ElectromagneticCryogenicJK2LBProperties : public ElectromagneticSolidProperties
{
public:
  static InputParameters validParams();

  ElectromagneticCryogenicJK2LBProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  // Electrical resistivity
  virtual Real electrical_resistivity_from_T(const Real & T) const override;
  virtual void
  electrical_resistivity_from_T(const Real & T, Real & rho, Real & drho_dT) const override;

  // Electrical conductivity
  virtual Real electrical_conductivity_from_T(const Real & T) const override;
  virtual void
  electrical_conductivity_from_T(const Real & T, Real & sigma, Real & dsigma_dT) const override;

  // Magnetic permeability
  virtual Real magnetic_permeability_from_T(const Real & T) const override;
  virtual void
  magnetic_permeability_from_T(const Real & T, Real & mu, Real & dmu_dT) const override;

#pragma GCC diagnostic pop

protected:
  /// Valid temperature range
  const Real _T_min;
  const Real _T_max;

  /// Electrical resistivity coefficients: rho(T) = c0 + c1*T + ... + c5*T^5 [Ohm·m]
  /// Quintic polynomial from Lu et al. (2009)
  /// Fitted via scipy.optimize.curve_fit to digitized data
  /// R^2 = 0.9987490979, RMSE = 8.03e-10 Ohm·m, Max Error = 2.25e-9 Ohm·m
  const Real _rho_c0;
  const Real _rho_c1;
  const Real _rho_c2;
  const Real _rho_c3;
  const Real _rho_c4;
  const Real _rho_c5;

  /// Magnetic permeability: constant mu_0 [H/m]
  /// Austenitic stainless steel is non-magnetic (antiferromagnetic below 240K)
  const Real _mu_const;
};
