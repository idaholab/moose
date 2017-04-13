/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef GRAVITY_H
#define GRAVITY_H

#include "Kernel.h"

class Function;
class Gravity;

template <>
InputParameters validParams<Gravity>();

/**
 * Gravity computes the body force (force/volume) given the acceleration of gravity (value) and the
 * density
 */
class Gravity : public Kernel
{
public:
  Gravity(const InputParameters & parameters);

  virtual ~Gravity() {}

protected:
  virtual Real computeQpResidual();

  const MaterialProperty<Real> & _density;

  Real _value;
  Function & _function;
  // _alpha parameter for HHT time integration scheme
  const Real _alpha;
};

#endif // GRAVITY_H
