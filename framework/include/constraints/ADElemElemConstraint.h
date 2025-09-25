//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "ElemElemConstraintBase.h"

/**
 * An ADElemElemConstraint is the AD (Automatic Differentiation) version of ElemElemConstraint
 * used for creating constraints between elements across an interface using automatic
 * differentiation to compute the Jacobian contributions.
 */
class ADElemElemConstraint : public ElemElemConstraintBase
{
public:
  static InputParameters validParams();

  ADElemElemConstraint(const InputParameters & parameters);

  /**
   * Computes the residual for this element or the neighbor
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type) override;

  /**
   * Computes the residual for the current side.
   */
  virtual void computeResidual() override;

  /**
   * Computes the element/neighbor-element/neighbor Jacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type) override;

  /**
   * Computes the jacobian for the current side.
   */
  virtual void computeJacobian() override;

protected:
  /**
   *  Compute the residual for one of the constraint quadrature points.  Must be overwritten by
   * derived class.
   */
  virtual ADReal computeQpResidual(Moose::DGResidualType type) = 0;

  /**
   *  Compute the Jacobian for one of the constraint quadrature points.
   */
  virtual Real computeQpJacobian(Moose::DGJacobianType type);

  /// Holds the AD variable value on the element side
  const ADVariableValue & _u;
  /// Holds the AD variable gradient on the element side
  const ADVariableGradient & _grad_u;
  /// Holds the AD variable value on the neighbor element side
  const ADVariableValue & _u_neighbor;
  /// Holds the AD variable gradient on the neighbor element side
  const ADVariableGradient & _grad_u_neighbor;
};
