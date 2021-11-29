//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExampleCoefDiffusion.h"

registerMooseObject("ExampleApp", ExampleCoefDiffusion);

InputParameters
ExampleCoefDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.set<Real>("coef") = 0.0;
  return params;
}

ExampleCoefDiffusion::ExampleCoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _coef(getParam<Real>("coef"))
{
}

Real
ExampleCoefDiffusion::computeQpResidual()
{
  return _coef * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
ExampleCoefDiffusion::computeQpJacobian()
{
  return _coef * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
