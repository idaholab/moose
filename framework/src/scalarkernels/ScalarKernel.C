//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarKernel.h"
#include "MooseVariableScalar.h"

InputParameters
ScalarKernel::validParams()
{
  InputParameters params = ScalarKernelBase::validParams();
  return params;
}

ScalarKernel::ScalarKernel(const InputParameters & parameters)
  : ScalarKernelBase(parameters), _u(_is_implicit ? _var.sln() : uOld())
{
}

void
ScalarKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  for (_i = 0; _i < _var.order(); _i++)
    _local_re(_i) += computeQpResidual();

  accumulateTaggedLocalResidual();
}

void
ScalarKernel::computeJacobian()
{
  prepareMatrixTag(_assembly, _var.number(), _var.number());

  for (_i = 0; _i < _local_ke.m(); _i++)
    _local_ke(_i, _i) += computeQpJacobian();

  accumulateTaggedLocalMatrix();
}
