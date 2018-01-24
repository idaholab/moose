//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef POLYFORCING_H_
#define POLYFORCING_H_

#include "Kernel.h"

class PolyForcing;

template <>
InputParameters validParams<PolyForcing>();

class PolyForcing : public Kernel
{
public:
  PolyForcing(const InputParameters & parameters);

protected:
  Real f();

  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  Real _x;
  Real _y;
  Real _z;
};

#endif // POLYFORCING_H
