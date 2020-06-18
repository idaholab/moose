//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Constraint.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseMesh.h"
#include "MooseVariableInterface.h"
#include "MortarInterface.h"
#include "TwoMaterialPropertyInterface.h"

// Forward Declarations
class MortarConstraintBase;
class FEProblemBase;
namespace libMesh
{
class QBase;
}

template <>
InputParameters validParams<MortarConstraintBase>();

/**
 * User for mortar methods
 *
 * Indexing:
 *
 *              T_m             T_s         lambda
 *         +--------------+-------------+-------------+
 * T_m     |  K_1         |             | SecondaryPrimary |
 *         +--------------+-------------+-------------+
 * T_s     |              |  K_2        | SecondarySecondary  |
 *         +--------------+-------------+-------------+
 * lambda  | PrimaryPrimary | PrimarySecondary |             |
 *         +--------------+-------------+-------------+
 *
 */
class MortarConstraintBase : public Constraint,
                             public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                             public MortarInterface,
                             public TwoMaterialPropertyInterface,
                             public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  MortarConstraintBase(const InputParameters & parameters);

  /**
   * Method for computing the residual
   * @param has_primary Whether the mortar segment element projects onto the primary face
   */
  virtual void computeResidual(bool has_primary);

  /**
   * Method for computing the Jacobian
   * @param has_primary Whether the mortar segment element projects onto the primary face
   */
  virtual void computeJacobian(bool has_primary);

  /**
   * compute the residual for the specified element type
   */
  virtual void computeResidual(Moose::MortarType mortar_type) = 0;

  /**
   * compute the residual for the specified element type
   */
  virtual void computeJacobian(Moose::MortarType mortar_type) = 0;

  /**
   * The variable number that this object operates on.
   */
  const MooseVariable * variable() const { return _var; }

  /**
   * Whether to use dual mortar
   */
  bool useDual() const { return _use_dual; }

private:
  /// Reference to the finite element problem
  FEProblemBase & _fe_problem;

protected:
  /// Pointer to the lagrange multipler variable. nullptr if none
  const MooseVariable * const _var;

  /// Reference to the secondary variable
  const MooseVariable & _secondary_var;

  /// Reference to the primary variable
  const MooseVariable & _primary_var;

private:
  /// Whether to compute primal residuals
  const bool _compute_primal_residuals;

  /// Whether to compute lagrange multiplier residuals
  const bool _compute_lm_residuals;

  /// A dummy object useful for constructing _test when not using Lagrange multipliers
  const VariableTestValue _test_dummy;

protected:
  /// Whether the current mortar segment projects onto a face on the primary side
  bool _has_primary;

  /// Whether to use the dual motar approach
  const bool _use_dual;

  /// the normals along the secondary face
  const MooseArray<Point> & _normals;

  /// the tangents along the secondary face
  const MooseArray<std::vector<Point>> & _tangents;

  /// The element Jacobian times weights
  const std::vector<Real> & _JxW_msm;

  /// Member for handling change of coordinate systems (xyz, rz, spherical)
  const MooseArray<Real> & _coord;

  /// The quadrature rule
  const QBase * const & _qrule_msm;

  /// The shape functions corresponding to the lagrange multiplier variable
  const VariableTestValue & _test;

  /// The shape functions corresponding to the secondary interior primal variable
  const VariableTestValue & _test_secondary;

  /// The shape functions corresponding to the primary interior primal variable
  const VariableTestValue & _test_primary;

  /// The shape function gradients corresponding to the secondary interior primal variable
  const VariableTestGradient & _grad_test_secondary;

  /// The shape function gradients corresponding to the primary interior primal variable
  const VariableTestGradient & _grad_test_primary;

  /// The locations of the quadrature points on the interior secondary elements
  const MooseArray<Point> & _phys_points_secondary;

  /// The locations of the quadrature points on the interior primary elements
  const MooseArray<Point> & _phys_points_primary;
};

#define usingMortarConstraintBaseMembers                                                           \
  usingConstraintMembers;                                                                          \
  using ADMortarConstraint::_phys_points_secondary;                                                \
  using ADMortarConstraint::_phys_points_primary;                                                  \
  using ADMortarConstraint::_has_primary;                                                          \
  using ADMortarConstraint::_use_dual;                                                             \
  using ADMortarConstraint::_test;                                                                 \
  using ADMortarConstraint::_test_secondary;                                                       \
  using ADMortarConstraint::_test_primary;                                                         \
  using ADMortarConstraint::_grad_test_secondary;                                                  \
  using ADMortarConstraint::_grad_test_primary;                                                    \
  using ADMortarConstraint::_normals;                                                              \
  using ADMortarConstraint::_tangents;                                                             \
  using ADMortarConstraint::_secondary_var;                                                        \
  using ADMortarConstraint::_primary_var
