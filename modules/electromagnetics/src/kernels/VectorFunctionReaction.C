//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorFunctionReaction.h"

registerMooseObject("ElectromagneticsApp", VectorFunctionReaction);

InputParameters
VectorFunctionReaction::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $sign * f * u$, where $f$ "
      "is a function coefficient and $u$ is a vector variable.");
  MooseEnum sign("positive=1 negative=-1", "positive");
  params.addParam<MooseEnum>("sign", sign, "Sign of boundary term in weak form.");
  params.addParam<FunctionName>("function", "1.0", "Function coefficient multiplier for field.");
  return params;
}

VectorFunctionReaction::VectorFunctionReaction(const InputParameters & parameters)
  : VectorKernel(parameters), _sign(getParam<MooseEnum>("sign")), _function(getFunction("function"))
{
}

Real
VectorFunctionReaction::computeQpResidual()
{
  return _sign * _function.value(_t, _q_point[_qp]) * _u[_qp] * _test[_i][_qp];
}

Real
VectorFunctionReaction::computeQpJacobian()
{
  return _sign * _function.value(_t, _q_point[_qp]) * _phi[_j][_qp] * _test[_i][_qp];
}
