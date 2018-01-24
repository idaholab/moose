//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MMSCONVECTION_H_
#define MMSCONVECTION_H_

#include "Kernel.h"

class MMSConvection;

template <>
InputParameters validParams<MMSConvection>();

class MMSConvection : public Kernel
{
public:
  MMSConvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  RealVectorValue velocity;

  Real _x;
  Real _y;
  Real _z;
};

#endif // MMSCONVECTION_H_
