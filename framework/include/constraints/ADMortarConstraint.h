//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarConstraintBase.h"

class ADMortarConstraint : public MortarConstraintBase
{
public:
  static InputParameters validParams();

  ADMortarConstraint(const InputParameters & parameters);

protected:
  /**
   * compute the residual at the quadrature points
   */
  virtual ADReal computeQpResidual(Moose::MortarType mortar_type) = 0;

  using MortarConstraintBase::computeResidual;
  /**
   * compute the residual for the specified element type
   */
  void computeResidual(Moose::MortarType mortar_type) override;

  using MortarConstraintBase::computeJacobian;
  /**
   * compute the residual for the specified element type
   */
  void computeJacobian(Moose::MortarType mortar_type) override;

  void computeResidualAndJacobian() override;

private:
  /// A dummy object useful for constructing _lambda when not using Lagrange multipliers
  const ADVariableValue _lambda_dummy;

protected:
  /// The LM solution
  const ADVariableValue & _lambda;

  /// The primal solution on the secondary side
  const ADVariableValue & _u_secondary;

  /// The primal solution on the primary side
  const ADVariableValue & _u_primary;

  /// The primal solution gradient on the secondary side
  const ADVariableGradient & _grad_u_secondary;

  /// The primal solution gradient on the primary side
  const ADVariableGradient & _grad_u_primary;
};
