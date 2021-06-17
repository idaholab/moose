//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalKernelBase.h"

/**
 * Base class for creating nodal kernels with AD-computed Jacobians
 */
class ADNodalKernel : public NodalKernelBase
{
public:
  /**
   * Class constructor.
   * @param parameters The InputParameters for the object
   */
  static InputParameters validParams();

  ADNodalKernel(const InputParameters & parameters);

  /**
   * Compute the residual at the current node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpResidual()
   */
  void computeResidual() override;

  /**
   * Compute the Jacobian at one node.
   *
   * Note: This is NOT what a user would normally want to override.
   * Usually a user would override computeQpJacobian()
   */
  void computeJacobian() override;

  /**
   * This method simply routes to computeJacobian whenever jvar == _var.number() since global AD
   * computes all the derivatives for all variables at once
   */
  void computeOffDiagJacobian(unsigned int jvar) override final;

protected:
  /**
   * The user can override this function to compute the residual at a node.
   */
  virtual ADReal computeQpResidual() = 0;

  /// Value of the unknown variable this is acting on
  const ADVariableValue & _u;
};
