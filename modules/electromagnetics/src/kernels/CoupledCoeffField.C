//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledCoeffField.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", CoupledCoeffField);

InputParameters
CoupledCoeffField::validParams()
{
  InputParameters params = ADCoupledForce::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $cfv$, where $c$ and $f$ are constant "
      "and function coefficients, respectively, and $v$ is a coupled scalar variable.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for field.");
  params.addParam<Real>("sign", 1.0, "Sign of term in weak form.");
  return params;
}

CoupledCoeffField::CoupledCoeffField(const InputParameters & parameters)
  : ADCoupledForce(parameters),

    _func(getFunction("func")),

    _sign(getParam<Real>("sign"))

{
}

ADReal
CoupledCoeffField::computeQpResidual()
{
  return _sign * _func.value(_t, _q_point[_qp]) * -1.0 * ADCoupledForce::computeQpResidual();
}
