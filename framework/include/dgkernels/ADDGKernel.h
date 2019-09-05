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

#include "DualRealOps.h"

#define usingDGKernelMembers                                                                       \
  usingDGKernelBaseMembers;                                                                        \
  usingUserObjectInterfaceMembers;                                                                 \
  using ADDGKernel<compute_stage>::_test;                                                          \
  using ADDGKernel<compute_stage>::_qp;                                                            \
  using ADDGKernel<compute_stage>::_i;                                                             \
  using ADDGKernel<compute_stage>::_u;                                                             \
  using ADDGKernel<compute_stage>::_var;                                                           \
  using ADDGKernel<compute_stage>::_grad_test;                                                     \
  using ADDGKernel<compute_stage>::_grad_u;                                                        \
  using ADDGKernel<compute_stage>::_current_elem;                                                  \
  using ADDGKernel<compute_stage>::_current_side_elem;                                             \
  using ADDGKernel<compute_stage>::_normals;                                                       \
  using ADDGKernel<compute_stage>::_grad_u_neighbor;                                               \
  using ADDGKernel<compute_stage>::_u_neighbor;                                                    \
  using ADDGKernel<compute_stage>::_test_neighbor;                                                 \
  using ADDGKernel<compute_stage>::_grad_test_neighbor

template <ComputeStage compute_stage>
class ADDGKernel;

declareADValidParams(ADDGKernel);

template <ComputeStage compute_stage>
class ADDGKernel : public DGKernelBase, public NeighborMooseVariableInterface<Real>
{
public:
  ADDGKernel(const InputParameters & parameters);

  virtual ~ADDGKernel();

  // See KernelBase base for documentation of these overridden methods
  virtual void computeResidual() override;
  virtual void computeElemNeighResidual(Moose::DGResidualType type) override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(unsigned int jvar) override;
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type) override;
  virtual void computeOffDiagElemNeighJacobian(Moose::DGJacobianType type,
                                               unsigned int jvar) override;

  virtual MooseVariableFEBase & variable() override { return _var; }

protected:
  /// Compute this Kernel's contribution to the residual at the current quadrature point
  virtual ADReal computeQpResidual(Moose::DGResidualType type) = 0;

  /// Variable this kernel operates on
  MooseVariable & _var;
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

  /// Holds the solution at current quadrature points
  const ADVariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const ADVariableGradient & _grad_u;

  /// Holds the current solution at the current quadrature point
  const ADVariableValue & _u_neighbor;

  /// Holds the current solution gradient at the current quadrature point
  const ADVariableGradient & _grad_u_neighbor;
};
