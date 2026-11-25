//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADLambdaU2.h"

registerMooseObject("MooseUnitApp", ADLambdaU2);

InputParameters
ADLambdaU2::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Kernel residual: lambda * u^2.");
  params.addRequiredParam<Real>("lambda", "Scalar coefficient.");
  return params;
}

ADLambdaU2::ADLambdaU2(const InputParameters & parameters)
  : ADKernel(parameters), _lambda(getParam<Real>("lambda"))
{
}

ADReal
ADLambdaU2::computeQpResidual()
{
  return _lambda * _u[_qp] * _u[_qp] * _test[_i][_qp];
}
