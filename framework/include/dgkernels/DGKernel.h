//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DGKernelBase.h"

/**
 * The DGKernel class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces).
 */
class DGKernel : public DGKernelBase, public NeighborMooseVariableInterface<Real>
{
public:
  /**
   * Factory constructor initializes all internal references needed for residual computation.
   *
   *
   * @param parameters The parameters object for holding additional parameters for kernels and
   * derived kernels
   */
  static InputParameters validParams();

  DGKernel(const InputParameters & parameters);

  /**
   * The variable that this kernel operates on.
   */
  virtual const MooseVariableFEBase & variable() const override { return _var; }

  /**
   * Computes the residual for this element or the neighbor
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type) override;

  /**
   * Computes the element/neighbor-element/neighbor Jacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type) override;

  /**
   * Computes the element-element off-diagonal Jacobian
   */
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                               const MooseVariableFEBase & jvar) override;

protected:
  /**
   * This is the virtual that derived classes should override for computing the residual on
   * neighboring element.
   */
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on
   * neighboring element.
   */
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the off-diag Jacobian.
   */
  virtual Real computeQpOffDiagJacobian(Moose::DGJacobianType type, unsigned int jvar);

  /**
   * Insertion point for evaluations that depend on qp but are independent of the test functions.
   */
  virtual void precalculateQpResidual(Moose::DGResidualType /*type*/) {}

  /**
   * Insertion point for evaluations that depend on qp but are independent of the test and shape
   * functions.
   */
  virtual void precalculateQpJacobian(Moose::DGJacobianType /*type*/) {}

  /**
   * Insertion point for evaluations that depend on qp but are independent of the test and shape
   * functions for off-diagonal Jacobian assembly.
   */
  virtual void precalculateQpOffDiagJacobian(Moose::DGJacobianType /*type*/,
                                             const MooseVariableFEBase & /*jvar*/)
  {
  }

  /// Variable this kernel operates on
  MooseVariable & _var;
  /// Holds the current solution at the current quadrature point on the face.
  const VariableValue & _u;
  /// Holds the current solution gradient at the current quadrature point on the face.
  const VariableGradient & _grad_u;
  /// Shape functions
  const VariablePhiValue & _phi;
  /// Gradient of shape function
  const VariablePhiGradient & _grad_phi;
  /// test functions
  const VariableTestValue & _test;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test;
  /// Side shape function
  const VariablePhiValue & _phi_neighbor;
  /// Gradient of side shape function
  const VariablePhiGradient & _grad_phi_neighbor;
  /// Side test function
  const VariableTestValue & _test_neighbor;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test_neighbor;
  /// Holds the current solution at the current quadrature point
  const VariableValue & _u_neighbor;
  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u_neighbor;
};
