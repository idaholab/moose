//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADArrayKernel.h"

/**
 * Test kernel for automatic differentiation coupling of array variables. Adds the residual
 * -(coef * (v + grad(v) . direction), test) where v is a coupled array variable.
 */
class ADArrayCoupledValueGradient : public ADArrayKernel
{
public:
  static InputParameters validParams();

  ADArrayCoupledValueGradient(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(ADRealEigenVector & residual) override;

  /// coupled array variable value
  const ADArrayVariableValue & _v;
  /// coupled array variable gradient
  const ADArrayVariableGradient & _grad_v;
  /// component-wise coefficient
  const RealEigenVector _coef;
  /// direction the gradient of v is projected on
  const RealVectorValue _direction;
};
