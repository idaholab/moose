//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementOptimizationFunctionInnerProduct.h"

class ElementOptimizationDiffusionCoefFunctionInnerProduct
  : public ElementOptimizationFunctionInnerProduct
{
public:
  static InputParameters validParams();

  ElementOptimizationDiffusionCoefFunctionInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpInnerProduct() override;
  /// Holds gradient of adjoint soln variable at the current quadrature points
  const VariableGradient & _grad_adjoint;
  /// Holds gradient of forward soln variable at the current quadrature points
  const VariableGradient & _grad_forward;
};
