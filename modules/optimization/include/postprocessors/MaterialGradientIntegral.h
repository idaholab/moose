//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementIntegralPostprocessor.h"
/**
 * This postprocessor computes the gradient for material inversion by taking the inner product of
 gradients of the forward and adjoint variables with material gradient
 */
class MaterialGradientIntegral : public ElementIntegralPostprocessor
{
public:
  static InputParameters validParams();

  MaterialGradientIntegral(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;
  /// Derivative of matererial with with respect to parameter being optimized
  const MaterialProperty<Real> & _dMdP;
  /// Holds gradient of adjoint soln variable at the current quadrature points
  const VariableGradient & _grad_u;
  /// Holds gradient of forward soln variable at the current quadrature points
  const VariableGradient & _grad_v;
};
