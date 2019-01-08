//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ADKERNEL_H
#define ADKERNEL_H

#include "KernelBase.h"

#define usingKernelMembers                                                                         \
  using ADKernel<compute_stage>::_test;                                                            \
  using ADKernel<compute_stage>::_qp;                                                              \
  using ADKernel<compute_stage>::_i;                                                               \
  using ADKernel<compute_stage>::_u;                                                               \
  using ADKernel<compute_stage>::_var;                                                             \
  using ADKernel<compute_stage>::_grad_test;                                                       \
  using ADKernel<compute_stage>::_grad_u;                                                          \
  using ADKernel<compute_stage>::_JxW;                                                             \
  using ADKernel<compute_stage>::_coord;                                                           \
  using ADKernel<compute_stage>::_local_re;                                                        \
  using ADKernel<compute_stage>::_qrule;                                                           \
  using ADKernel<compute_stage>::_save_in;                                                         \
  using ADKernel<compute_stage>::_has_save_in;                                                     \
  using ADKernel<compute_stage>::_current_elem_volume;                                             \
  using ADKernel<compute_stage>::coupled;                                                          \
  using ADKernel<compute_stage>::coupledComponents;                                                \
  using ADKernel<compute_stage>::getBlockCoordSystem;                                              \
  using ADKernel<compute_stage>::paramError;                                                       \
  using ADKernel<compute_stage>::isParamValid

template <ComputeStage compute_stage>
class ADKernel;

declareADValidParams(ADKernel);

template <ComputeStage compute_stage>
class ADKernel : public KernelBase, public MooseVariableInterface<Real>
{
public:
  ADKernel(const InputParameters & parameters);

  virtual ~ADKernel();

  // See KernelBase base for documentation of these overridden methods
  virtual void computeResidual() override;
  virtual void computeJacobian() override;
  virtual void computeOffDiagJacobian(MooseVariableFEBase & jvar) override;
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) override;

  virtual MooseVariable & variable() override { return _var; }

protected:
  /// Compute this Kernel's contribution to the residual at the current quadrature point
  virtual ADResidual computeQpResidual() = 0;

  /// This is a regular kernel so we cast to a regular MooseVariable
  MooseVariable & _var;

  /// the current test function
  const ADVariableTestValue & _test;

  /// gradient of the test function
  const ADVariableTestGradient & _grad_test;

  /// Holds the solution at current quadrature points
  const ADVariableValue & _u;

  /// Holds the solution gradient at the current quadrature points
  const ADVariableGradient & _grad_u;
};

#endif /* ADKERNEL_H */
