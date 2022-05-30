//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledFuncDiffusion.h"
#include "Function.h"

registerMooseObject("ElectromagneticsApp", CoupledFuncDiffusion);

InputParameters
CoupledFuncDiffusion::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription(
      "Represents a coupled Laplacian term with sign and function coefficients.");
  params.addParam<FunctionName>("func", 1.0, "Function multiplier for diffusion term.");
  params.addParam<Real>("sign", 1.0, "Sign of Kernel, if it needs to be changed.");
  params.addRequiredCoupledVar("coupled_field", "Coupled field variable.");
  return params;
}

CoupledFuncDiffusion::CoupledFuncDiffusion(const InputParameters & parameters)
  : Kernel(parameters),
    _func(getFunction("func")),
    _sign(getParam<Real>("sign")),
    _coupled_grad(coupledGradient("coupled_field"))
{
}

Real
CoupledFuncDiffusion::computeQpResidual()
{
  return _sign * _func.value(_t, _q_point[_qp]) * _grad_test[_i][_qp] * _coupled_grad[_qp];
}

Real
CoupledFuncDiffusion::computeQpJacobian()
{
  return 0;
}
