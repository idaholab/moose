/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef RICHARDSDENSITYCONSTBULK_H
#define RICHARDSDENSITYCONSTBULK_H

#include "RichardsDensity.h"

class RichardsDensityConstBulk;

template <>
InputParameters validParams<RichardsDensityConstBulk>();

/**
 * Fluid density assuming constant bulk modulus
 */
class RichardsDensityConstBulk : public RichardsDensity
{
public:
  RichardsDensityConstBulk(const InputParameters & parameters);

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
  /// density = _dens0*exp(p/_bulk)
  Real _dens0;

  /// density = _dens0*exp(p/_bulk)
  Real _bulk;
};

#endif // RICHARDSDENSITYCONSTBULK_H
