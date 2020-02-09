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
template <typename T>
class ADIntegratedBCTempl : public IntegratedBCBase, public MooseVariableInterface<T>
{
public:
  static InputParameters validParams();

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
  virtual ADReal computeQpResidual() = 0;

  /// The variable that this IntegratedBC operates on
  MooseVariableFE<T> & _var;

  /// normals at quadrature points
  const MooseArray<ADPoint> & _normals;

  /// (physical) quadrature points
  const MooseArray<ADPoint> & _ad_q_points;

  // test functions

  /// test function values (in QPs)
  const ADTemplateVariableTestValue<T> & _test;
  /// gradients of test functions  (in QPs)
  const ADTemplateVariableTestGradient<T> & _grad_test;

  /// the values of the unknown variable this BC is acting on
  const ADTemplateVariableValue<T> & _u;
  /// the gradient of the unknown variable this BC is acting on
  const ADTemplateVariableGradient<T> & _grad_u;

  /// The ad version of JxW
  const MooseArray<ADReal> & _ad_JxW;

  /// The AD version of coord
  const MooseArray<ADReal> & _ad_coord;

  /// Whether this object is acting on the displaced mesh
  const bool _use_displaced_mesh;
};

using ADIntegratedBC = ADIntegratedBCTempl<Real>;
using ADVectorIntegratedBC = ADIntegratedBCTempl<RealVectorValue>;
