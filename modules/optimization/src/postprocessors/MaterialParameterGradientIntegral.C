//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialParameterGradientIntegral.h"

registerMooseObject("OptimizationApp", MaterialParameterGradientIntegral);

InputParameters
MaterialParameterGradientIntegral::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addClassDescription(
      "Compute the gradient for material inversion by taking the inner product of gradients of the "
      "forward and adjoint variables with material gradient");

  params.addRequiredCoupledVar("adjoint_var", "Variable from adjoint solution");
  params.addRequiredCoupledVar("forward_var", "Variable from the forward solution");
  params.addRequiredParam<MaterialPropertyName>(
      "material_derivative", "Material Derivative w.r.t. parameter being optimized");
  return params;
}

MaterialParameterGradientIntegral::MaterialParameterGradientIntegral(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _dMdP(getMaterialProperty<Real>("material_derivative")),
    _grad_u(coupledGradient("adjoint_var")),
    _grad_v(coupledGradient("forward_var"))
{
}

Real
MaterialParameterGradientIntegral::computeQpIntegral()
{
  return -_grad_v[_qp] * _dMdP[_qp] * _grad_u[_qp];
}
