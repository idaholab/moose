//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MMSDIFFUSION_H_
#define MMSDIFFUSION_H_

#include "Kernel.h"

class MMSDiffusion;

template <>
InputParameters validParams<MMSDiffusion>();

class MMSDiffusion : public Kernel
{
public:
  MMSDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  unsigned int _mesh_dimension;
};

#endif // MMSDIFFUSION_H
