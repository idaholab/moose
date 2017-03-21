/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSDENSITYIDEAL_H
#define RICHARDSDENSITYIDEAL_H

#include "RichardsDensity.h"

class RichardsDensityIdeal;

template <>
InputParameters validParams<RichardsDensityIdeal>();

/**
 * Fluid density of an ideal gas
 */
class RichardsDensityIdeal : public RichardsDensity
{
public:
  RichardsDensityIdeal(const InputParameters & parameters);

  /**
   * fluid density as a function of porepressure
   * @param p porepressure
   */
  Real density(Real p) const;

  /**
   * derivative of fluid density wrt porepressure
   */
  Real ddensity(Real /*p*/) const;

  /**
   * second derivative of fluid density wrt porepressure
   */
  Real d2density(Real /*p*/) const;

protected:
  /// density = _slope*(p - _p0)
  Real _slope;

  /// density = _slope*(p - _p0)
  Real _p0;
};

#endif // RICHARDSDENSITYIDEAL_H
