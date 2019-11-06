//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoefDiffusion.h"

registerMooseObject("MooseTestApp", CoefDiffusion);

InputParameters
CoefDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addCustomTypeParam("coef", 0.0, "CoefficientType", "The coefficient of diffusion");
  params.addPrivateParam<Real>("_test_private_param", 12345);
  params.addParam<Real>("non_controllable", "A parameter we cannot control.");

  params.declareControllable("coef");

  return params;
}

CoefDiffusion::CoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _coef(getParam<Real>("coef"))
{
}

Real
CoefDiffusion::computeQpResidual()
{
  return _coef * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
CoefDiffusion::computeQpJacobian()
{
  return _coef * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
