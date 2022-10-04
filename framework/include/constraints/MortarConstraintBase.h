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
#include "MortarConsumerInterface.h"
#include "TwoMaterialPropertyInterface.h"

// Forward Declarations
class FEProblemBase;
namespace libMesh
{
class QBase;
}

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
                             public MortarConsumerInterface,
                             public TwoMaterialPropertyInterface,
                             public MooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  MortarConstraintBase(const InputParameters & parameters);

  /**
   * Method for computing the residual
   */
  virtual void computeResidual() override final;

  /**
   * Method for computing the Jacobian
   */
  virtual void computeJacobian() override final;

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
  const MooseVariable & variable() const override { return *_var; }

  /**
   * The variable number that this object operates on (pointer).
   */
  const MooseVariable * variablePtr() const { return _var; }

  /**
   * Whether to use dual mortar
   */
  bool useDual() const { return _use_dual; }

  /**
   * This method will be called after the loop over the mortar segment mesh
   */
  virtual void post() {}

  /**
   * This method will be called after the loop over the mortar segment mesh
   */
  virtual void incorrectEdgeDroppingPost(const std::unordered_set<const Node *> &) {}

  /**
   * A post routine for zeroing all inactive LM DoFs
   */
  void zeroInactiveLMDofs(const std::unordered_set<const Node *> & inactive_lm_nodes,
                          const std::unordered_set<const Elem *> & inactive_lm_elems);

protected:
  const FEProblemBase & feProblem() const { return _fe_problem; }

  /// Reference to the finite element problem
  FEProblemBase & _fe_problem;

  /// Pointer to the lagrange multipler variable. nullptr if none
  MooseVariable * const _var;

  /// Reference to the secondary variable
  MooseVariableField<Real> & _secondary_var;

  /// Reference to the primary variable
  MooseVariableField<Real> & _primary_var;

  /// Whether to compute primal residuals
  const bool _compute_primal_residuals;

  /// Whether to compute lagrange multiplier residuals
  const bool _compute_lm_residuals;

  /// A dummy object useful for constructing _test when not using Lagrange multipliers
  const VariableTestValue _test_dummy;

  /// Whether to use the dual motar approach
  const bool _use_dual;

  /// the normals along the primary face
  const MooseArray<Point> & _normals_primary;

  /// Tangent vectors on the secondary faces (libmesh)
  const MooseArray<std::vector<Point>> & _tangents;

  /// Member for handling change of coordinate systems (xyz, rz, spherical)
  const MooseArray<Real> & _coord;

  /// The quadrature points in physical space
  const std::vector<Point> & _q_point;

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

  /// the higher-dimensional secondary face element
  const Elem * const & _interior_secondary_elem;

  /// the higher-dimensional primary face element
  const Elem * const & _interior_primary_elem;

  /// the lower-dimensional secondary element
  const Elem * const & _lower_secondary_elem;

  /// The primary face lower dimensional element (not the mortar element!). The mortar element
  /// lives on the secondary side of the mortar interface and *may* correspond to \p
  /// _lower_secondary_elem under the very specific circumstance that the nodes on the primary side
  /// of the mortar interface exactly project onto the secondary side of the mortar interface. In
  /// general projection of primary nodes will split the face elements on the secondary side of the
  /// interface. It is these split elements that are the mortar segment mesh elements
  Elem const * const & _lower_primary_elem;

  /// Whether this object operates on the displaced mesh
  const bool _displaced;
};
