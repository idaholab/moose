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

class MortarConstraint : public MortarConstraintBase
{
public:
  static InputParameters validParams();

  MortarConstraint(const InputParameters & parameters);

  // Using declarations necessary to pull in computeResidual with different parameter list and avoid
  // hidden method warning
  using MortarConstraintBase::computeResidual;

  // Using declarations necessary to pull in computeJacobian with different parameter list and avoid
  // hidden method warning
  using MortarConstraintBase::computeJacobian;

protected:

  /**
   * compute the residual for primary/secondary/lower
   */
  virtual void computeResidual(Moose::MortarType mortar_type) override;

  /**
   * compute the scalar residual
   */
  virtual void computeResidualScalar() override;

  /**
   * compute the Jacobian for the specified element type
   */
  virtual void computeJacobian(Moose::MortarType mortar_type) override;

  /**
   * Method for computing the scalar Jacobian
   */
  virtual void computeJacobianScalar() override;


  /**
   * compute the residual at the quadrature points
   */
  virtual Real computeQpResidual(Moose::MortarType mortar_type) = 0;

  /**
   * compute the scalar residual at the quadrature points
   */
  virtual Real computeQpResidualScalar() {return 0;};
  virtual Real computeQpResidualScalarScalar() {return 0;};

  /**
   * compute the jacobian at the quadrature points
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType jacobian_type,
                                 unsigned int jvar) = 0;

  /**
   * compute the scalar jacobian at the quadrature points
   */
  virtual Real computeQpJacobianScalarScalar() {return 0;};

private:
  /// A dummy object useful for constructing _lambda when not using Lagrange multipliers
  const VariableValue _lambda_dummy;

protected:

  /// The LM solution
  const VariableValue & _lambda;

  /// The primal solution on the secondary side
  const VariableValue & _u_secondary;

  /// The primal solution on the primary side
  const VariableValue & _u_primary;

  /// The primal solution gradient on the secondary side
  const VariableGradient & _grad_u_secondary;

  /// The primal solution gradient on the primary side
  const VariableGradient & _grad_u_primary;

  /// The current shape functions
  const VariablePhiValue * _phi;

  /// The current shape function gradients
  const VariablePhiGradient * _grad_phi;

  /// The current shape functions for vector variables
  const VectorVariablePhiValue * _vector_phi;

  /// The current shape function gradients for vector variables
  const VectorVariablePhiGradient * _vector_grad_phi;
};
