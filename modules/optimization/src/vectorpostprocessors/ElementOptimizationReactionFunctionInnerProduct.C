//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementOptimizationReactionFunctionInnerProduct.h"

registerMooseObject("OptimizationApp", ElementOptimizationReactionFunctionInnerProduct);

InputParameters
ElementOptimizationReactionFunctionInnerProduct::validParams()
{
  InputParameters params = ElementOptimizationFunctionInnerProduct::validParams();
  params.addClassDescription(
      "Compute the gradient for reaction material inversion by taking the inner product of the "
      "forward and adjoint variables multiplied by the derivative of the optimization function "
      "with respect to the controllable parameters.");

  params.addRequiredCoupledVar("forward_variable", "Variable from the forward solution");
  return params;
}

ElementOptimizationReactionFunctionInnerProduct::ElementOptimizationReactionFunctionInnerProduct(
    const InputParameters & parameters)
  : ElementOptimizationFunctionInnerProduct(parameters),
    _forward_var(coupledValue("forward_variable"))
{
}

Real
ElementOptimizationReactionFunctionInnerProduct::computeQpInnerProduct()
{
  return -_var[_qp] * _forward_var[_qp];
}
