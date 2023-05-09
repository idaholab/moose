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

/**
 * ADInterfaceKernel and ADVectorInterfaceKernel is responsible for interfacing physics across
 * subdomains
 */
template <typename T>
class ADInterfaceKernelTempl : public InterfaceKernelBase, public NeighborMooseVariableInterface<T>
{
public:
  static InputParameters validParams();

  ADInterfaceKernelTempl(const InputParameters & parameters);

  /// The primary variable that this interface kernel operates on
  const MooseVariableFE<T> & variable() const override { return _var; }

  /// The neighbor variable number that this interface kernel operates on
  const MooseVariableFE<T> & neighborVariable() const override { return _neighbor_var; }

private:
  /**
   * Using the passed DGResidual type, selects the correct test function space and residual block,
   * and then calls computeQpResidual
   */
  void computeElemNeighResidual(Moose::DGResidualType type) override final;

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and
   * jacobian block, and then calls computeQpJacobian
   */
  void computeElemNeighJacobian(Moose::DGJacobianType type) override final;

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and
   * jacobian block, and then calls computeQpOffDiagJacobian with the passed jvar
   */
  void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                       unsigned int jvar) override final;

  /// Selects the correct Jacobian type and routine to call for the primary variable jacobian
  void computeElementOffDiagJacobian(unsigned int jvar) override final;

  /// Selects the correct Jacobian type and routine to call for the secondary variable jacobian
  void computeNeighborOffDiagJacobian(unsigned int jvar) override final;

  /// Computes the residual for the current side.
  void computeResidual() override final;

  /// Computes the jacobian for the current side.
  void computeJacobian() override final;

protected:
  /// Compute residuals at quadrature points
  virtual ADReal computeQpResidual(Moose::DGResidualType type) = 0;

  /**
   * Put necessary evaluations depending on qp but independent on test functions here
   */
  virtual void initQpResidual(Moose::DGResidualType /* type */) {}

  /// The primary side MooseVariable
  MooseVariableFE<T> & _var;

  /// Normal vectors at the quadrature points
  const MooseArray<Point> & _normals;

  /// Holds the current solution at the current quadrature point on the face.
  const ADTemplateVariableValue<T> & _u;

  /// Holds the current solution gradient at the current quadrature point on the face.
  const ADTemplateVariableGradient<T> & _grad_u;

  /// The ad version of JxW
  const MooseArray<ADReal> & _ad_JxW;

  /// The ad version of coord
  const MooseArray<ADReal> & _ad_coord;

  /// The ad version of q_point
  const MooseArray<ADPoint> & _ad_q_point;

  /// shape function
  const ADTemplateVariablePhiValue<T> & _phi;

  /// Side shape function.
  const ADTemplateVariableTestValue<T> & _test;

  /// Gradient of side shape function
  const ADTemplateVariableTestGradient<T> & _grad_test;

  /// Coupled neighbor variable
  const MooseVariableFE<T> & _neighbor_var;

  /// Coupled neighbor variable value
  const ADTemplateVariableValue<T> & _neighbor_value;

  /// Coupled neighbor variable gradient
  const ADTemplateVariableGradient<T> & _grad_neighbor_value;

  /// Side neighbor shape function.
  const ADTemplateVariablePhiValue<T> & _phi_neighbor;

  /// Side neighbor test function
  const ADTemplateVariableTestValue<T> & _test_neighbor;

  /// Gradient of side neighbor shape function
  const ADTemplateVariableTestGradient<T> & _grad_test_neighbor;

  /// Holds residual entries as they are accumulated by this InterfaceKernel
  /// This variable is temporarily reserved for RattleSnake
  DenseMatrix<Number> _local_kxx;

private:
  Moose::DGResidualType resid_type;
};

typedef ADInterfaceKernelTempl<Real> ADInterfaceKernel;
typedef ADInterfaceKernelTempl<RealVectorValue> ADVectorInterfaceKernel;
