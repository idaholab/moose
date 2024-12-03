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

#define TemplateVariableValue typename OutputTools<T>::VariableValue
#define TemplateVariableGradient typename OutputTools<T>::VariableGradient
#define TemplateVariablePhiValue typename OutputTools<T>::VariablePhiValue
#define TemplateVariablePhiGradient typename OutputTools<T>::VariablePhiGradient
#define TemplateVariableTestValue typename OutputTools<T>::VariableTestValue
#define TemplateVariableTestGradient typename OutputTools<T>::VariableTestGradient

// Forward Declarations
template <typename T>
class InterfaceKernelTempl;

typedef InterfaceKernelTempl<Real> InterfaceKernel;
typedef InterfaceKernelTempl<RealVectorValue> VectorInterfaceKernel;

/**
 * InterfaceKernel and VectorInterfaceKernel is responsible for interfacing physics across
 * subdomains
 */
template <typename T>
class InterfaceKernelTempl : public InterfaceKernelBase, public NeighborMooseVariableInterface<T>
{
public:
  static InputParameters validParams();

  InterfaceKernelTempl(const InputParameters & parameters);

  /// The primary variable that this interface kernel operates on
  virtual const MooseVariableFE<T> & variable() const override { return _var; }

  /// The neighbor variable number that this interface kernel operates on
  virtual const MooseVariableFE<T> & neighborVariable() const override { return _neighbor_var; }

  /**
   * Using the passed DGResidual type, selects the correct test function space and residual block,
   * and then calls computeQpResidual
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type);

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and jacobian block, and then calls computeQpJacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type);

  /**
   * Using the passed DGJacobian type, selects the correct test function and trial function spaces
   * and
   * jacobian block, and then calls computeQpOffDiagJacobian with the passed jvar
   */
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type, unsigned int jvar);

  /// Selects the correct Jacobian type and routine to call for the primary variable jacobian
  virtual void computeElementOffDiagJacobian(unsigned int jvar) override;

  /// Selects the correct Jacobian type and routine to call for the secondary variable jacobian
  virtual void computeNeighborOffDiagJacobian(unsigned int jvar) override;

  /// Computes the residual for the current side.
  virtual void computeResidual() override;

  /// Computes the jacobian for the current side.
  virtual void computeJacobian() override;

  /// Computes the residual and Jacobian for the current side.
  virtual void computeResidualAndJacobian() override;

  /// Compute residuals at quadrature points
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;

  /**
   * Put necessary evaluations depending on qp but independent on test functions here
   */
  virtual void initQpResidual(Moose::DGResidualType /* type */) {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here
   */
  virtual void initQpJacobian(Moose::DGJacobianType /* type */) {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here for
   * off-diagonal Jacobian assembly
   */
  virtual void initQpOffDiagJacobian(Moose::DGJacobianType /* type */, unsigned int /* jvar */) {}

protected:
  /// The primary side MooseVariable
  MooseVariableFE<T> & _var;

  /// Normal vectors at the quadrature points
  const MooseArray<Point> & _normals;

  /// Holds the current solution at the current quadrature point on the face.
  const TemplateVariableValue & _u;

  /// Holds the current solution gradient at the current quadrature point on the face.
  const TemplateVariableGradient & _grad_u;

  /// shape function
  const TemplateVariablePhiValue & _phi;

  /// Shape function gradient
  const TemplateVariablePhiGradient & _grad_phi;

  /// Side shape function.
  const TemplateVariableTestValue & _test;

  /// Gradient of side shape function
  const TemplateVariableTestGradient & _grad_test;

  /// Coupled neighbor variable
  const MooseVariableFE<T> & _neighbor_var;

  /// Coupled neighbor variable value
  const TemplateVariableValue & _neighbor_value;

  /// Coupled neighbor variable gradient
  const TemplateVariableGradient & _grad_neighbor_value;

  /// Side neighbor shape function.
  const TemplateVariablePhiValue & _phi_neighbor;

  /// Gradient of side neighbor shape function
  const TemplateVariablePhiGradient & _grad_phi_neighbor;

  /// Side neighbor test function
  const TemplateVariableTestValue & _test_neighbor;

  /// Gradient of side neighbor shape function
  const TemplateVariableTestGradient & _grad_test_neighbor;

  /// Whether the variable and the neighbor variables are part of the same system
  /// (whether from two different nonlinear systems or a nonlinear and an auxiliary system)
  const bool _same_system;

  /// Holds residual entries as they are accumulated by this InterfaceKernel
  /// This variable is temporarily reserved for RattleSnake
  DenseMatrix<Number> _local_kxx;
};
