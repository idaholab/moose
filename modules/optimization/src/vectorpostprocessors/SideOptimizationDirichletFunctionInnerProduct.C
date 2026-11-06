//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideOptimizationDirichletFunctionInnerProduct.h"

registerMooseObject("OptimizationApp", SideOptimizationDirichletFunctionInnerProduct);

InputParameters
SideOptimizationDirichletFunctionInnerProduct::validParams()
{
  InputParameters params = SideOptimizationFunctionInnerProduct::validParams();
  params.addClassDescription(
      "Computes the gradient for Dirichlet boundary condition inversion by taking the inner "
      "product of the normal diffusive flux of the adjoint variable with the derivative of the "
      "optimization function with respect to the controllable parameters.");
  params.addRequiredParam<MaterialPropertyName>(
      "diffusivity",
      "The name of the diffusivity material property that will be used in the flux computation.");
  return params;
}

SideOptimizationDirichletFunctionInnerProduct::SideOptimizationDirichletFunctionInnerProduct(
    const InputParameters & parameters)
  : SideOptimizationFunctionInnerProduct(parameters),
    _grad_var(coupledGradient("variable")),
    _diffusivity(getMaterialProperty<Real>("diffusivity"))
{
  if (getFieldVar("variable", 0)->isFV())
    paramError("variable", "This object only supports finite element variables.");
}

Real
SideOptimizationDirichletFunctionInnerProduct::computeQpInnerProduct()
{
  return -_diffusivity[_qp] * (_grad_var[_qp] * _normals[_qp]);
}
