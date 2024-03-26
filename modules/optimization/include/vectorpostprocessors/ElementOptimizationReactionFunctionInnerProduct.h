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
/**
 * Compute the gradient for reaction material inversion by taking the inner product of the forward
 * and adjoint variables multiplied by the derivative of the optimization function with respect to
 * the controllable parameters.
 */
class ElementOptimizationReactionFunctionInnerProduct
  : public ElementOptimizationFunctionInnerProduct
{
public:
  static InputParameters validParams();

  ElementOptimizationReactionFunctionInnerProduct(const InputParameters & parameters);

protected:
  virtual Real computeQpInnerProduct() override;
  /// Holds forward solution variable at the current quadrature points
  const VariableValue & _forward_var;
};
