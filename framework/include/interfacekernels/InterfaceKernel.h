//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// local includes
#include "InterfaceKernelBase.h"

// Forward Declarations
class InterfaceKernel;

template <>
InputParameters validParams<InterfaceKernel>();

/**
 * InterfaceKernel is responsible for interfacing physics across subdomains
 */

class InterfaceKernel : public InterfaceKernelBase, public NeighborMooseVariableInterface<Real>
{
public:
  InterfaceKernel(const InputParameters & parameters);

  /// The master variable that this interface kernel operates on
  virtual MooseVariable & variable() const override { return _var; }

  /// The neighbor variable number that this interface kernel operates on
  virtual const MooseVariable & neighborVariable() const override { return _neighbor_var; }

  /**
   * Using the passed DGResidual type, selects the correct test function space and residual block,
   * and then calls computeQpResidual
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type) override;

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and
   * jacobian block, and then calls computeQpJacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type) override;

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and
   * jacobian block, and then calls computeQpOffDiagJacobian with the passed jvar
   */
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                               unsigned int jvar) override;

  /// Selects the correct Jacobian type and routine to call for the master variable jacobian
  virtual void computeElementOffDiagJacobian(unsigned int jvar) override;

  /// Selects the correct Jacobian type and routine to call for the slave variable jacobian
  virtual void computeNeighborOffDiagJacobian(unsigned int jvar) override;

  /// Computes the residual for the current side.
  virtual void computeResidual() override;

  /// Computes the jacobian for the current side.
  virtual void computeJacobian() override;

  /// Compute residuals at quadrature points
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;

protected:
  /// The master side MooseVariable
  MooseVariable & _var;

  /// Normal vectors at the quadrature points
  const MooseArray<Point> & _normals;

  /// Holds the current solution at the current quadrature point on the face.
  const VariableValue & _u;

  /// Holds the current solution gradient at the current quadrature point on the face.
  const VariableGradient & _grad_u;

  /// shape function
  const VariablePhiValue & _phi;

  /// Shape function gradient
  const VariablePhiGradient & _grad_phi;

  /// Side shape function.
  const VariableTestValue & _test;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test;

  /// Coupled neighbor variable
  MooseVariable & _neighbor_var;

  /// Coupled neighbor variable value
  const VariableValue & _neighbor_value;

  /// Coupled neighbor variable gradient
  const VariableGradient & _grad_neighbor_value;

  /// Side neighbor shape function.
  const VariablePhiValue & _phi_neighbor;
  /// Gradient of side neighbor shape function
  const VariablePhiGradient & _grad_phi_neighbor;

  /// Side neighbor test function
  const VariableTestValue & _test_neighbor;
  /// Gradient of side neighbor shape function
  const VariableTestGradient & _grad_test_neighbor;

  /// Holds residual entries as they are accumulated by this InterfaceKernel
  /// This variable is temporarily reserved for RattleSnake
  DenseMatrix<Number> _local_kxx;
};
