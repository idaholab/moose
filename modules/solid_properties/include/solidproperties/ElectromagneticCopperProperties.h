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
 * Electromagnetic properties of oxygen-free high-conductivity (OFHC) copper.
 *
 * Electrical resistivity from NIST Monograph 177, Chapter 8, equation (8-1):
 * Simon, N. J., Drexler, E. S., and Reed, R. P. (1992).
 * Properties of Copper and Copper Alloys at Cryogenic Temperatures.
 *
 * Three-component resistivity model: rho(T) = rho_0 + rho_i + rho_i0
 * where rho_0 = rho(273K) / RRR (residual resistivity),
 * rho_i = ideal/lattice resistivity,
 * rho_i0 = deviation from Matthiessen's rule.
 *
 * Valid temperature range: 2-900 K
 */
class ElectromagneticCopperProperties : public ElectromagneticSolidProperties
{
public:
  static InputParameters validParams();

  ElectromagneticCopperProperties(const InputParameters & parameters);

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

  // Magnetic permeability (constant for copper)
  virtual Real magnetic_permeability_from_T(const Real & T) const override;
  virtual void
  magnetic_permeability_from_T(const Real & T, Real & mu, Real & dmu_dT) const override;

#pragma GCC diagnostic pop

protected:
  /// Valid temperature range
  const Real _T_min;
  const Real _T_max;

  /// Residual resistivity ratio (user parameter, default 100)
  const Real _rrr;

  /// Residual resistivity rho_0 = 1.553e-8 / RRR [Ohm*m]
  const Real _rho_0;

  /// NIST copper electrical resistivity coefficients (SI units)
  const Real _P1;    // 1.171e-17 [Ohm*m * K^(-P2)]
  const Real _P2;    // 4.49 [dimensionless]
  const Real _P3;    // 3.841e10 [(Ohm*m)^(-1) * K^P4]
  const Real _P4;    // 1.14 [dimensionless]
  const Real _P5;    // 50 [K]
  const Real _P6;    // 6.428 [dimensionless]
  const Real _P7;    // 0.4531 [dimensionless]
  const Real _rho_c; // 0.0 [Ohm*m] (copper-specific correction)

  /// Magnetic permeability (constant: mu_0 = 4*pi * 10^-7 H/m)
  const Real _mu_const;

private:
  /**
   * Helper function to compute electrical resistivity components from NIST correlation
   * PLACEHOLDER - Will implement NIST equation (8-1) when user provides analytical expressions
   * Computes rho_i(T) and rho_i0(T) in nOhm*m, then converts to Ohm*m
   */
  void computeElectricalResistivity(
      Real T, Real & rho_i, Real & rho_i0, Real & drho_i_dT, Real & drho_i0_dT) const;
};
