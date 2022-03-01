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

#include "MooseVariableScalar.h"

/**
 * The DGKernel class is responsible for calculating the residuals for various
 * physics on internal sides (edges/faces).
 */
class ArrayDGKernel : public DGKernelBase, public NeighborMooseVariableInterface<RealEigenVector>
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

  ArrayDGKernel(const InputParameters & parameters);

  /**
   * The variable that this kernel operates on.
   */
  virtual const MooseVariableFEBase & variable() const override { return _var; }

  /**
   * Override this function to consider couplings of components of the array variable
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

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
   * neighboring element. Residual to be filled in \p residual.
   */
  virtual void computeQpResidual(Moose::DGResidualType type, RealEigenVector & residual) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on
   * neighboring element.
   */
  virtual RealEigenVector computeQpJacobian(Moose::DGJacobianType);

  /**
   * This is the virtual that derived classes should override for computing the off-diag Jacobian.
   */
  virtual RealEigenMatrix computeQpOffDiagJacobian(Moose::DGJacobianType type,
                                                   const MooseVariableFEBase & jvar);

  /**
   * Put necessary evaluations depending on qp but independent on test functions here
   */
  virtual void initQpResidual(Moose::DGResidualType) {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here
   */
  virtual void initQpJacobian(Moose::DGJacobianType) {}

  /**
   * Put necessary evaluations depending on qp but independent on test and shape functions here for
   * off-diagonal Jacobian assembly
   */
  virtual void initQpOffDiagJacobian(Moose::DGJacobianType, const MooseVariableFEBase &) {}

  /// Variable this kernel operates on
  ArrayMooseVariable & _var;
  /// Holds the current solution at the current quadrature point on the face.
  const ArrayVariableValue & _u;
  /// Holds the current solution gradient at the current quadrature point on the face.
  const ArrayVariableGradient & _grad_u;
  /// Shape functions
  const ArrayVariablePhiValue & _phi;
  /// Gradient of shape function
  const ArrayVariablePhiGradient & _grad_phi;
  const MappedArrayVariablePhiGradient & _array_grad_phi;
  /// test functions
  const ArrayVariableTestValue & _test;
  /// Gradient of side shape function
  const ArrayVariableTestGradient & _grad_test;
  const MappedArrayVariablePhiGradient & _array_grad_test;
  /// Side shape function
  const ArrayVariablePhiValue & _phi_neighbor;
  /// Gradient of side shape function
  const ArrayVariablePhiGradient & _grad_phi_neighbor;
  const MappedArrayVariablePhiGradient & _array_grad_phi_neighbor;
  /// Side test function
  const ArrayVariableTestValue & _test_neighbor;
  /// Gradient of side shape function
  const ArrayVariableTestGradient & _grad_test_neighbor;
  const MappedArrayVariablePhiGradient & _array_grad_test_neighbor;
  /// Holds the current solution at the current quadrature point
  const ArrayVariableValue & _u_neighbor;
  /// Holds the current solution gradient at the current quadrature point
  const ArrayVariableGradient & _grad_u_neighbor;
  /// Normals in type of Eigen map
  const std::vector<Eigen::Map<RealDIMValue>> & _array_normals;
  /// Number of components of the array variable
  const unsigned int _count;

private:
  /// Work vector for residual computation
  RealEigenVector _work_vector;
};
