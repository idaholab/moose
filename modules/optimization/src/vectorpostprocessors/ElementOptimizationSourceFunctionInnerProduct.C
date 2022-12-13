//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementOptimizationSourceFunctionInnerProduct.h"

registerMooseObject("OptimizationApp", ElementOptimizationSourceFunctionInnerProduct);

InputParameters
ElementOptimizationSourceFunctionInnerProduct::validParams()
{
  InputParameters params = ElementOptimizationFunctionInnerProduct::validParams();
  params.addClassDescription("Computes the inner product of variable with parameterized source "
                             "function for optimization gradient computation.");
  return params;
}

ElementOptimizationSourceFunctionInnerProduct::ElementOptimizationSourceFunctionInnerProduct(
    const InputParameters & parameters)
  : ElementOptimizationFunctionInnerProduct(parameters)
{
}

Real
ElementOptimizationSourceFunctionInnerProduct::computeQpInnerProduct()
{
  return _var[_qp];
}
