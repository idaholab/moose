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

InputParameters
ODETimeKernel::validParams()
{
  InputParameters params = ODEKernel::validParams();

  params.set<MultiMooseEnum>("vector_tags") = "time";
  params.set<MultiMooseEnum>("matrix_tags") = "system time";

  return params;
}

ODETimeKernel::ODETimeKernel(const InputParameters & parameters)
  : ODEKernel(parameters), _u_dot(_var.uDot()), _du_dot_du(_var.duDotDu())
{
}

void
ODETimeKernel::computeResidual()
{
  prepareVectorTag(_assembly, _var.number());

  for (_i = 0; _i < _var.order(); _i++)
    _local_re(_i) += computeQpResidual();

  accumulateTaggedLocalResidual();
}
