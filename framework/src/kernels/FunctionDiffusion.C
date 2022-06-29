//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionDiffusion.h"
#include "Function.h"

registerMooseObject("MooseApp", FunctionDiffusion);

InputParameters
FunctionDiffusion::validParams()
{
  InputParameters params = Diffusion::validParams();
  params.addClassDescription("The Laplacian operator with a function coefficient.");
  params.addParam<FunctionName>("function", 1.0, "Function multiplier for diffusion term.");
  params.addCoupledVar("v",
                       "Coupled concentration variable for kernel to operate on; if this "
                       "is not specified, the kernel's nonlinear variable will be used as "
                       "usual");
  return params;
}

FunctionDiffusion::FunctionDiffusion(const InputParameters & parameters)
  : Diffusion(parameters),
    _function(getFunction("function")),
    _grad_v(isCoupled("v") ? coupledGradient("v") : _grad_u),
    _v_var(isCoupled("v") ? getVar("v", 0) : nullptr),
    _grad_v_phi(isCoupled("v") ? _v_var->gradPhi() : _grad_phi)
{
}

Real
FunctionDiffusion::computeQpResidual()
{
  return _function.value(_t, _q_point[_qp]) * _grad_v[_qp] * _grad_test[_i][_qp];
}

Real
FunctionDiffusion::computeQpJacobian()
{
  return _function.value(_t, _q_point[_qp]) * _grad_v_phi[_j][_qp] * _grad_test[_i][_qp];
}
