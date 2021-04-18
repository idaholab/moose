//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledVectorCoeffField.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", CoupledVectorCoeffField);

InputParameters
CoupledVectorCoeffField::validParams()
{
  InputParameters params = VectorKernel::validParams();
  params.addClassDescription(
      "Kernel representing the contribution of the PDE term $cfv$, where $c$ and $f$ are constant "
      "and function coefficients, respectively, and $v$ is a coupled vector variable.");
  params.addParam<Real>("coeff", 1.0, "Coefficient multiplier for field.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for field.");
  params.addRequiredCoupledVar("coupled", "Coupled variable.");
  return params;
}

CoupledVectorCoeffField::CoupledVectorCoeffField(const InputParameters & parameters)
  : VectorKernel(parameters),

    _coefficient(getParam<Real>("coeff")),

    _func(getFunction("func")),

    _coupled_val(coupledVectorValue("coupled"))
{
}

Real
CoupledVectorCoeffField::computeQpResidual()
{
  return _coefficient * _func.value(_t, _q_point[_qp]) * _coupled_val[_qp] * _test[_i][_qp];
}

Real
CoupledVectorCoeffField::computeQpJacobian()
{
  return 0.0;
}
