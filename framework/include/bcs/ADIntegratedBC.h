//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IntegratedBCBase.h"
#include "MooseVariableInterface.h"

/**
 * Base class for deriving any boundary condition of a integrated type
 */
template <typename T, ComputeStage compute_stage>
class ADIntegratedBCTempl : public IntegratedBCBase, public MooseVariableInterface<T>
{
public:
  ADIntegratedBCTempl(const InputParameters & parameters);

  virtual MooseVariableFE<T> & variable() override { return _var; }

  void computeResidual() override;
  void computeJacobian() override;
  void computeJacobianBlock(MooseVariableFEBase & jvar) override;
  void computeJacobianBlockScalar(unsigned int jvar) override;

protected:
  /**
   * Compute this IntegratedBC's contribution to the residual at the current quadrature point
   */
  virtual ADResidual computeQpResidual() = 0;

  /// The variable that this IntegratedBC operates on
  MooseVariableFE<T> & _var;

  /// normals at quadrature points
  const typename PointType<compute_stage>::type & _normals;

  /// (physical) quadrature points
  const typename PointType<compute_stage>::type & _ad_q_points;

  // test functions

  /// test function values (in QPs)
  const ADTemplateVariableTestValue & _test;
  /// gradients of test functions  (in QPs)
  const typename VariableTestGradientType<T, compute_stage>::type & _grad_test;

  /// the values of the unknown variable this BC is acting on
  const ADTemplateVariableValue & _u;
  /// the gradient of the unknown variable this BC is acting on
  const ADTemplateVariableGradient & _grad_u;

  /// The ad version of JxW
  const MooseArray<typename Moose::RealType<compute_stage>::type> & _ad_JxW;
};

template <ComputeStage compute_stage>
using ADIntegratedBC = ADIntegratedBCTempl<Real, compute_stage>;
template <ComputeStage compute_stage>
using ADVectorIntegratedBC = ADIntegratedBCTempl<RealVectorValue, compute_stage>;

declareADValidParams(ADIntegratedBC);
declareADValidParams(ADVectorIntegratedBC);

#define usingTemplIntegratedBCMembers(type)                                                        \
  using ADIntegratedBCTempl<type, compute_stage>::_test;                                           \
  using ADIntegratedBCTempl<type, compute_stage>::_qp;                                             \
  using ADIntegratedBCTempl<type, compute_stage>::_i;                                              \
  using ADIntegratedBCTempl<type, compute_stage>::_u;                                              \
  using ADIntegratedBCTempl<type, compute_stage>::_var;                                            \
  using ADIntegratedBCTempl<type, compute_stage>::_grad_test;                                      \
  using ADIntegratedBCTempl<type, compute_stage>::_grad_u;                                         \
  using ADIntegratedBCTempl<type, compute_stage>::_dt;                                             \
  using ADIntegratedBCTempl<type, compute_stage>::_current_elem;                                   \
  using ADIntegratedBCTempl<type, compute_stage>::_t;                                              \
  using ADIntegratedBCTempl<type, compute_stage>::_q_point;                                        \
  using ADIntegratedBCTempl<type, compute_stage>::_assembly;                                       \
  using ADIntegratedBCTempl<type, compute_stage>::_local_ke;                                       \
  using ADIntegratedBCTempl<type, compute_stage>::_j;                                              \
  using ADIntegratedBCTempl<type, compute_stage>::_JxW;                                            \
  using ADIntegratedBCTempl<type, compute_stage>::_coord;                                          \
  using ADIntegratedBCTempl<type, compute_stage>::_qrule;                                          \
  using ADIntegratedBCTempl<type, compute_stage>::_normals;                                        \
  using ADIntegratedBCTempl<type, compute_stage>::getFunction;                                     \
  using ADIntegratedBCTempl<type, compute_stage>::_ad_q_points

#define usingIntegratedBCMembers usingTemplIntegratedBCMembers(Real)
#define usingVectorIntegratedBCMembers usingTemplIntegratedBCMembers(RealVectorValue)

