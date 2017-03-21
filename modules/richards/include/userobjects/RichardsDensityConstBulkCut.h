/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSDENSITYCONSTBULKCUT_H
#define RICHARDSDENSITYCONSTBULKCUT_H

#include "RichardsDensity.h"

class RichardsDensityConstBulkCut;

template <>
InputParameters validParams<RichardsDensityConstBulkCut>();

/**
 * Fluid density assuming constant bulk modulus, for p>cut_limit
 * Then following a cubic for zero_point <= p <= cut_limit
 * Then zero for p<zero_point.
 * The cubic is chosen so the function and its first derivative is continuous throughout
 */
class RichardsDensityConstBulkCut : public RichardsDensity
{
public:
  RichardsDensityConstBulkCut(const InputParameters & parameters);

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
  /// density = _dens0*exp(p/_bulk), modified by cubic
  Real _dens0;

  /// density = _dens0*exp(p/_bulk), modified by cubic
  Real _bulk;

  /// where the cubic starts
  Real _cut_limit;

  /// where the density is zero
  Real _zero_point;

  /// (cut_limit-zero_point)^3
  Real _c3;
};

#endif // RICHARDSDENSITYCONSTBULKCUT_H
