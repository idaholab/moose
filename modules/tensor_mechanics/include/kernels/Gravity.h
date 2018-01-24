//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
