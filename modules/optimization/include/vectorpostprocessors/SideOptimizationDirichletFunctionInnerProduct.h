//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SideOptimizationFunctionInnerProduct.h"

class SideOptimizationDirichletFunctionInnerProduct : public SideOptimizationFunctionInnerProduct
{
public:
  static InputParameters validParams();
  SideOptimizationDirichletFunctionInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpInnerProduct() override;

  /// Holds gradient of the coupled adjoint variable at the current quadrature points
  const VariableGradient & _grad_var;
  /// Diffusivity used to form the normal diffusive flux of the adjoint variable
  const MaterialProperty<Real> & _diffusivity;
};
