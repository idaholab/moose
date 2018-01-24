//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef DGCONVECTION_H
#define DGCONVECTION_H

#include "DGKernel.h"

class DGConvection;

template <>
InputParameters validParams<DGConvection>();

class DGConvection : public DGKernel
{
public:
  DGConvection(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type);
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  RealVectorValue _velocity;
};

#endif // DGCONVECTION_H
