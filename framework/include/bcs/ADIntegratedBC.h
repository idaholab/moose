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
#include "ADFunctorInterface.h"

/**
 * Base class for deriving any boundary condition of a integrated type
 */
template <typename T>
class ADIntegratedBCTempl : public IntegratedBCBase,
                            public MooseVariableInterface<T>,
                            public ADFunctorInterface
{
public:
  static InputParameters validParams();

  ADIntegratedBCTempl(const InputParameters & parameters);

  const MooseVariableFE<T> & variable() const override { return _var; }

protected:
  void computeResidual() override;
  void computeJacobian() override;
  void computeResidualAndJacobian() override;
  void computeOffDiagJacobian(unsigned int jvar) override;
  void computeOffDiagJacobianScalar(unsigned int jvar) override;

  /**
   * compute the \p _residuals member for filling the Jacobian. We want to calculate these residuals
   * up-front when doing loal derivative indexing because we can use those residuals to fill \p
   * _local_ke for every associated jvariable. We do not want to re-do these calculations for every
   * jvariable and corresponding \p _local_ke. For global indexing we will simply pass the computed
   * \p _residuals directly to \p Assembly::addJacobian
   */
  virtual void computeResidualsForJacobian();

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

  /// The current shape functions
  const ADTemplateVariablePhiValue<T> & _phi;

  /// Whether this object is acting on the displaced mesh
  const bool _use_displaced_mesh;

  /// Data members for holding residuals
  ADReal _r;
  std::vector<Real> _residuals;
  std::vector<ADReal> _residuals_and_jacobians;

private:
  /**
   * compute all the Jacobian entries
   */
  void computeADJacobian();
};

using ADIntegratedBC = ADIntegratedBCTempl<Real>;
using ADVectorIntegratedBC = ADIntegratedBCTempl<RealVectorValue>;
