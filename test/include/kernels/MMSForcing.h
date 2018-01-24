//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MMSFORCING_H_
#define MMSFORCING_H_

#include "Kernel.h"

class MMSForcing;

template <>
InputParameters validParams<MMSForcing>();

class MMSForcing : public Kernel
{
public:
  MMSForcing(const InputParameters & parameters);

protected:
  Real f();

  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

private:
  unsigned int _mesh_dimension;

  Real _x;
  Real _y;
  Real _z;
};

#endif // MMSFORCING_H_
