//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FuncCoefDiffusion.h"

registerMooseObject("MooseTestApp", FuncCoefDiffusion);

InputParameters
FuncCoefDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addParam<FunctionName>("coef", "0.5*x+0.5*y", "The function for conductivity");
  return params;
}

FuncCoefDiffusion::FuncCoefDiffusion(const InputParameters & parameters)
  : Kernel(parameters), _function(getFunction("coef"))
{
}

Real
FuncCoefDiffusion::computeQpResidual()
{
  Real k = _function.value(_t, _q_point[_qp]);
  return k * _grad_test[_i][_qp] * _grad_u[_qp];
}

Real
FuncCoefDiffusion::computeQpJacobian()
{
  Real k = _function.value(_t, _q_point[_qp]);
  return k * _grad_test[_i][_qp] * _grad_phi[_j][_qp];
}
