/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GAUSSCONTFORCING_H
#define GAUSSCONTFORCING_H

#include "Kernel.h"

class GaussContForcing;

template <>
InputParameters validParams<GaussContForcing>();

/**
 * Note: This class is duplicated from moose_test.  It is useful for testing
 */
class GaussContForcing : public Kernel
{
public:
  GaussContForcing(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const Real _amplitude;
  const Real _x_center;
  const Real _y_center;
  const Real _z_center;

  const Real _x_spread;
  const Real _y_spread;
  const Real _z_spread;

  const Real _x_min;
  const Real _x_max;
  const Real _y_min;
  const Real _y_max;
  const Real _z_min;
  const Real _z_max;
};

#endif // GAUSSCONTFORCING_H
