//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

class MMSForcing : public Kernel
{
public:
  static InputParameters validParams();

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
