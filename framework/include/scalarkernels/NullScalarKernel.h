//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarKernel.h"

/**
 * Scalar kernel that sets a zero residual, to avoid error from system missing this variable.
 */
class NullScalarKernel : public ScalarKernel
{
public:
  static InputParameters validParams();

  NullScalarKernel(const InputParameters & parameters);

  virtual void reinit() override;

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;

  /// filler value to put on the on-diagonal Jacobian
  const Real _jacobian_fill;

  /// Local index
  unsigned int _i;
};
