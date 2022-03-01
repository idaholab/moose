//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UserForcingFunctionNodalKernel.h"

#include "Function.h"

registerMooseObject("MooseApp", UserForcingFunctionNodalKernel);

InputParameters
UserForcingFunctionNodalKernel::validParams()
{
  InputParameters params = NodalKernel::validParams();
  params.addClassDescription(
      "Residual contribution to an ODE from a source function acting at nodes.");
  params.addRequiredParam<FunctionName>("function", "The forcing function");
  return params;
}

UserForcingFunctionNodalKernel::UserForcingFunctionNodalKernel(const InputParameters & parameters)
  : NodalKernel(parameters), _func(getFunction("function"))
{
}

Real
UserForcingFunctionNodalKernel::computeQpResidual()
{
  return -_func.value(_t, (*_current_node));
}
