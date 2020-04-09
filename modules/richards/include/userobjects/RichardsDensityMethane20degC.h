//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RichardsDensity.h"

/**
 * Methane density - a quadratic fit to expressions in:
 * "Results of (pressure, density, temperature) measurements on methane and on nitrogen in the
 * temperature range from 273.15K to 323.15K at pressures up to 12MPa using new apparatus for
 * accurate gas-density"
 * This is only valid for p>=0, which is the physical region.  I extend to the p>0 domain with an
 * exponential, which will probably be sampled as the newton interative process converges towards
 * the solution.
 * NOTE: this expression is only valid to about P=20MPa.  Use van der Waals (RichardsDensityVDW) for
 * higher pressures.
 */
class RichardsDensityMethane20degC : public RichardsDensity
{
public:
  static InputParameters validParams();

  RichardsDensityMethane20degC(const InputParameters & parameters);

  /**
   * fluid density as a function of porepressure
   * @param p porepressure
   */
  Real density(Real p) const;

  /**
   * derivative of fluid density wrt porepressure
   * @param p porepressure
   */
  Real ddensity(Real p) const;

  /**
   * second derivative of fluid density wrt porepressure
   * @param p porepressure
   */
  Real d2density(Real p) const;

private:
  /// Unit of measurement for pressure (should be 1 for pressure in Pa, 1E6 for pressure in MPa, etc)
  Real _p_unit;
};
