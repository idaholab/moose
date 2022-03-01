//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

/**
 * All time kernels should inherit from this class
 *
 */
class TimeKernel : public Kernel
{
public:
  static InputParameters validParams();

  TimeKernel(const InputParameters & parameters);

  virtual void computeResidual() override;

  /**
   * Entry point for additional computation for the local residual after the standard calls
   *
   * NOTE: This is an advanced interface and in nearly all cases you should just override
   * computeQpResidual()!
   */
  virtual void computeResidualAdditional() {}

protected:
  /// Time derivative of u
  const VariableValue & _u_dot;

  /// Derivative of u_dot with respect to u
  const VariableValue & _du_dot_du;
};
