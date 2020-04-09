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
 * Density of a gas according to the van der Waals expression
 * (P + n^2 a/V^2)(V - nb) = nRT
 * How density is calculated: given P, (1/V) is calculated for n=1
 * and rho = molar_mass*(1/V).
 * The density is actually modified from this rho in the following ways:
 *  (1) density = rho - rho(P=0).  This is so that density(P=0)=0, which is
 *      physically correct, but the wan der Waals expression does not hold
 *      close to a vacuum.  However, it can be vital that density(P=0)=0
 *      numerically, otherwise fluid can be withdrawn from a node where there
 *      shouldn't be any fluid at P=0.  This is a miniscule correction, of
 *      many orders of magnitude below the precision of a, b, R or T
 *  (2) For P<0 we enter an unphysical region, but this may be sampled
 *      by the Newton process as the algorithm iterates towards the solution.
 *      Therefore i set density = infinity_ratio*molar_mass*(exp(-c*P) - 1) in this region
 *      where c is chosen so the derivatives match at P=0
 */
class RichardsDensityVDW : public RichardsDensity
{
public:
  static InputParameters validParams();

  RichardsDensityVDW(const InputParameters & parameters);

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

protected:
  /// van der Waals a
  Real _a;

  /// van der Waals b
  Real _b;

  /// R*T (gas constant * temperature)
  Real _rt;

  /// molar mass of gas
  Real _molar_mass;

  /// density at P=-infinity is _infinity_ratio*_molar_mass
  Real _infinity_ratio;

  /// R*T*b/a
  Real _rhs;

  /// b^2/a
  Real _b2oa;

  /// density at P=0 according to the van der Waals expression
  Real _vdw0;

  /// (1/_molar_mass)*d(density)/dP at P=0
  Real _slope0;

  /**
   * Density according to the van der Waals expression
   * This is modified to yield density(p) as noted above
   * @param p porepressure
   */
  Real densityVDW(Real p) const;
};
