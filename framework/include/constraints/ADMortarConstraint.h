//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MortarConstraintBase.h"

class ADMortarConstraint : public MortarConstraintBase
{
public:
  static InputParameters validParams();

  ADMortarConstraint(const InputParameters & parameters);

protected:
  /**
   * compute the residual at the quadrature points
   */
  virtual ADReal computeQpResidual(Moose::MortarType mortar_type) = 0;

  using MortarConstraintBase::computeResidual;
  /**
   * compute the residual for the specified element type
   */
  void computeResidual(Moose::MortarType mortar_type) override;

  using MortarConstraintBase::computeJacobian;
  /**
   * compute the residual for the specified element type
   */
  void computeJacobian(Moose::MortarType mortar_type) override;

  void computeResidualAndJacobian() override;

  /**
   * @return The AD mortar segment JxW times the coordinate transformation at the current
   * quadrature point, which carries the motion of the mortar segment geometry. Only valid when
   * \p _ad_segment_geometry is true.
   */
  ADReal adJxWCoord() const;

private:
  /// A dummy object useful for constructing _lambda when not using Lagrange multipliers
  const ADVariableValue _lambda_dummy;

  /// An empty gradient for constructing _grad_u_secondary and _grad_u_primary when the derived
  /// class does not use the primal gradients
  const ADVariableGradient _grad_u_dummy;

protected:
  /// The LM solution
  const ADVariableValue & _lambda;

  /// The primal solution on the secondary side
  const ADVariableValue & _u_secondary;

  /// The primal solution on the primary side
  const ADVariableValue & _u_primary;

  /// The primal solution gradient on the secondary side. Empty unless the private parameter
  /// _compute_primal_gradients is true.
  const ADVariableGradient & _grad_u_secondary;

  /// The primal solution gradient on the primary side. Empty unless the private parameter
  /// _compute_primal_gradients is true.
  const ADVariableGradient & _grad_u_primary;

  /**
   * The Lagrange multiplier, secondary, and primary test functions at the current _i and _qp that
   * carry the motion of the mortar segment quadrature points. Only valid when
   * \p _ad_segment_geometry is true; otherwise use _test, _test_secondary, and _test_primary.
   * Derived classes that support segment_geometry_derivatives use these and set the private
   * parameter _supports_segment_geometry_derivatives to true.
   */
  const ADReal & adTest() const;
  const ADReal & adTestSecondary() const;
  const ADReal & adTestPrimary() const;

private:
  /// The AD test functions filled by Assembly at the mortar segment quadrature points
  const MooseArray<std::vector<ADReal>> * _ad_test = nullptr;
  const MooseArray<std::vector<ADReal>> * _ad_test_secondary = nullptr;
  const MooseArray<std::vector<ADReal>> * _ad_test_primary = nullptr;
};

inline const ADReal &
ADMortarConstraint::adTest() const
{
  mooseAssert(_ad_segment_geometry && _ad_test, "The AD test functions are not set");
  return (*_ad_test)[_i][_qp];
}

inline const ADReal &
ADMortarConstraint::adTestSecondary() const
{
  mooseAssert(_ad_segment_geometry, "The AD test functions are not set");
  return (*_ad_test_secondary)[_i][_qp];
}

inline const ADReal &
ADMortarConstraint::adTestPrimary() const
{
  mooseAssert(_ad_segment_geometry, "The AD test functions are not set");
  return (*_ad_test_primary)[_i][_qp];
}
