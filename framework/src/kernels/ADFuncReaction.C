//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFuncReaction.h"
#include "Function.h"

registerMooseObject("MooseApp", ADFuncReaction);

InputParameters
ADFuncReaction::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $fu$, where $f$ is a "
      "function coefficient, and $u$ is a scalar variable.");
  params.addParam<FunctionName>("func", 1.0, "Function coefficient.");
  return params;
}

ADFuncReaction::ADFuncReaction(const InputParameters & parameters)
  : ADKernel(parameters), _func(getFunction("func"))
{
}

ADReal
ADFuncReaction::computeQpResidual()
{
  return _func.value(_t, _q_point[_qp]) * _test[_i][_qp] * _u[_qp];
}
