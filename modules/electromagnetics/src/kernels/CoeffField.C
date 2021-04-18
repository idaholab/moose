//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoeffField.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", CoeffField);

InputParameters
CoeffField::validParams()
{
  InputParameters params = Reaction::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $cfu$, where $c$ and $f$ are constant "
      "and function coefficients, respectively, and $u$ is a scalar variable.");
  params.addParam<Real>("coeff", 1.0, "Coefficient multiplier for field.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for field.");
  return params;
}

CoeffField::CoeffField(const InputParameters & parameters)
  : Reaction(parameters),

    _coefficient(getParam<Real>("coeff")),

    _func(getFunction("func"))

{
}

Real
CoeffField::computeQpResidual()
{
  return -_coefficient * _func.value(_t, _q_point[_qp]) * Reaction::computeQpResidual();
}

Real
CoeffField::computeQpJacobian()
{
  return -_coefficient * _func.value(_t, _q_point[_qp]) * Reaction::computeQpJacobian();
}
