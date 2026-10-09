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
 * Mechanical properties of oxygen-free high-conductivity (OFHC) copper.
 *
 * Young's modulus and Poisson's ratio from NIST Monograph 177:
 * Simon, N. J., Drexler, E. S., and Reed, R. P. (1992).
 * Properties of Copper and Copper Alloys at Cryogenic Temperatures.
 *
 * Thermal expansion coefficient from NIST Cryogenic Materials Database:
 * https://trc.nist.gov/cryogenics/materials/OFHC%20Copper/OFHC_Copper_rev1.htm
 *
 * Valid temperature range: 4-300 K
 */
class MechanicalCopperProperties : public MechanicalSolidProperties
{
public:
  static InputParameters validParams();

  MechanicalCopperProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  // Young's modulus
  virtual Real E_from_T(const Real & T) const override;
  virtual void E_from_T(const Real & T, Real & E, Real & dE_dT) const override;

  // Poisson's ratio
  virtual Real nu_from_T(const Real & T) const override;
  virtual void nu_from_T(const Real & T, Real & nu, Real & dnu_dT) const override;

  // Coefficient of thermal expansion
  virtual Real alpha_from_T(const Real & T) const override;
  virtual void alpha_from_T(const Real & T, Real & alpha, Real & dalpha_dT) const override;

#pragma GCC diagnostic pop

protected:
  /// Valid temperature range
  const Real _T_min;
  const Real _T_max;

  /// Young's modulus coefficients (NIST Monograph 177, p. 6-1)
  /// E(T) = 1e9 × (137 - 1.27e-4 × T²) [Pa]
  const Real _E_c0;
  const Real _E_c2;

  /// Poisson's ratio coefficients (NIST Monograph 177, p. 6-23)
  /// ν(T) = 0.339 + 7.03e-8 × T²
  const Real _nu_c0;
  const Real _nu_c2;

  /// Thermal expansion coefficient (NIST Cryogenic Database)
  /// log₁₀(α [10⁻⁶/K]) = Σ cᵢ × [log₁₀(T)]ⁱ for i=0..6
  const Real _alpha_c0, _alpha_c1, _alpha_c2, _alpha_c3, _alpha_c4, _alpha_c5, _alpha_c6;
};
