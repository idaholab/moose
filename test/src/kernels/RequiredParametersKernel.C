//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RequiredParametersKernel.h"

registerMooseObject("MooseTestApp", RequiredParametersKernel);

template <>
InputParameters
validParams<RequiredParametersKernel>()
{
  InputParameters params = validParams<CoefDiffusion>();
  // parameter defined in CoefDiffusion without default value
  // MOOSE should fail if the parameter is missing in the input file
  params.makeParamRequired<Real>("non_controllable");
  return params;
}

RequiredParametersKernel::RequiredParametersKernel(const InputParameters & parameters)
  : CoefDiffusion(parameters)
{
}

Real
RequiredParametersKernel::computeQpResidual()
{
  return _coef * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
RequiredParametersKernel::computeQpJacobian()
{
  return _coef * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
