//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideOptimizationNeumannFunctionInnerProduct.h"

registerMooseObject("OptimizationApp", SideOptimizationNeumannFunctionInnerProduct);

InputParameters
SideOptimizationNeumannFunctionInnerProduct::validParams()
{
  InputParameters params = SideOptimizationFunctionInnerProduct::validParams();
  params.addClassDescription("Computes the inner product of variable with parameterized Neumann "
                             "function for optimization gradient computation.");
  return params;
}

SideOptimizationNeumannFunctionInnerProduct::SideOptimizationNeumannFunctionInnerProduct(
    const InputParameters & parameters)
  : SideOptimizationFunctionInnerProduct(parameters)
{
}

Real
SideOptimizationNeumannFunctionInnerProduct::computeQpInnerProduct()
{
  return _var[_qp];
}
