//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementOptimizationDiffusionCoefFunctionInnerProduct.h"

registerMooseObject("OptimizationApp", ElementOptimizationDiffusionCoefFunctionInnerProduct);

InputParameters
ElementOptimizationDiffusionCoefFunctionInnerProduct::validParams()
{
  InputParameters params = ElementOptimizationFunctionInnerProduct::validParams();
  params.addClassDescription(
      "Compute the gradient for material inversion by taking the inner product of gradients of the "
      "forward and adjoint variables with material gradient");

  params.addRequiredCoupledVar("forward_variable", "Variable from the forward solution");
  return params;
}

ElementOptimizationDiffusionCoefFunctionInnerProduct::
    ElementOptimizationDiffusionCoefFunctionInnerProduct(const InputParameters & parameters)
  : ElementOptimizationFunctionInnerProduct(parameters),
    _grad_adjoint(coupledGradient("variable")),
    _grad_forward(coupledGradient("forward_variable"))
{
}

Real
ElementOptimizationDiffusionCoefFunctionInnerProduct::computeQpInnerProduct()
{
  return -_grad_adjoint[_qp] * _grad_forward[_qp];
}
