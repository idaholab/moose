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
 * Test kernel for the automatic differentiation time derivatives of array variables. Adds the
 * residual (a u_dot + b u_dotdot, test) where u_dot and u_dotdot are the time derivatives of
 * a coupled array variable v, or of the kernel variable if v is not given.
 */
class ADArrayCoupledTimeDerivatives : public ADArrayKernel
{
public:
  static InputParameters validParams();

  ADArrayCoupledTimeDerivatives(const InputParameters & parameters);

protected:
  virtual void computeQpResidual(ADRealEigenVector & residual) override;

  /// first time derivative, only requested for a nonzero coefficient so that the second time
  /// derivative can be requested on its own
  const ADArrayVariableValue * const _dot;
  /// second time derivative
  const ADArrayVariableValue & _dotdot;
  /// coefficient of the first time derivative
  const Real _a;
  /// coefficient of the second time derivative
  const Real _b;
};
