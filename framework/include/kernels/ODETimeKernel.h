//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ODEKernel.h"

/**
 * Base class for ODEKernels that contribute to the time residual
 * vector.
 */
class ODETimeKernel : public ODEKernel
{
public:
  static InputParameters validParams();

  ODETimeKernel(const InputParameters & parameters);

  virtual void computeResidual() override;

protected:
  /// Time derivative of u
  const VariableValue & _u_dot;

  /// Derivative of u_dot wrt u
  const VariableValue & _du_dot_du;
};
