//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADFuncCoupledForce.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", ADFuncCoupledForce);

InputParameters
ADFuncCoupledForce::validParams()
{
  InputParameters params = ADCoupledForce::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $-cfv$, where $c$ and $f$ are constant "
      "and function coefficients, respectively, and $v$ is a coupled scalar variable.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for the coupled force term.");
  return params;
}

ADFuncCoupledForce::ADFuncCoupledForce(const InputParameters & parameters)
  : ADCoupledForce(parameters), _func(getFunction("func"))
{
}

ADReal
ADFuncCoupledForce::computeQpResidual()
{
  return _func.value(_t, _q_point[_qp]) * ADCoupledForce::computeQpResidual();
}
