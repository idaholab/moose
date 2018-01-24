//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ODETimeKernel.h"

// MOOSE includes
#include "Assembly.h"
#include "MooseVariableScalar.h"
#include "SystemBase.h"

template <>
InputParameters
validParams<ODETimeKernel>()
{
  InputParameters params = validParams<ODEKernel>();
  return params;
}

ODETimeKernel::ODETimeKernel(const InputParameters & parameters) : ODEKernel(parameters) {}

void
ODETimeKernel::computeResidual()
{
  DenseVector<Number> & re = _assembly.residualBlock(_var.number(), Moose::KT_TIME);
  for (_i = 0; _i < _var.order(); _i++)
    re(_i) += computeQpResidual();
}
