//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

#include <unordered_map>

/**
 * Computes the normal contact mortar constraints for dynamic simulations.
 */
class ComputeDynamicWeightedGapLMMechanicalContact : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  ComputeDynamicWeightedGapLMMechanicalContact(const InputParameters & parameters);

protected:
  /**
   * Computes properties that are functions only of the current quadrature point (\p _qp), e.g.
   * indepedent of shape functions
   */
  virtual void computeQpProperties();

  virtual void computeQpIProperties();

  ADReal computeQpResidual(Moose::MortarType mortar_type) final;

  using ADMortarConstraint::computeResidual;
  void computeResidual(Moose::MortarType mortar_type) override;
  using ADMortarConstraint::computeJacobian;
  void computeJacobian(Moose::MortarType mortar_type) override;
  void residualSetup() override;
  void jacobianSetup() override final;

  void timestepSetup() override;

  virtual void post() override;

  virtual void
  incorrectEdgeDroppingPost(const std::unordered_set<const Node *> & inactive_lm_nodes) override;

  void communicateWear();

  /**
   * Method called from \p post(). Used to enforce node-associated constraints. E.g. for the base \p
   * ComputeWeightedGapLMMechanicalContact we enforce the zero-penetration constraint in this method
   * using an NCP function. This is also where we actually feed the node-based constraint
   * information into the system residual and Jacobian
   */
  virtual void enforceConstraintOnDof(const DofObject * const dof);

  /// x-displacement on the secondary face
  const ADVariableValue & _secondary_disp_x;
  /// x-displacement on the primary face
  const ADVariableValue & _primary_disp_x;
  /// y-displacement on the secondary face
  const ADVariableValue & _secondary_disp_y;
  /// y-displacement on the primary face
  const ADVariableValue & _primary_disp_y;

  /// For 2D mortar contact no displacement will be specified, so const pointers used
  const bool _has_disp_z;
  /// z-displacement on the secondary face
  const ADVariableValue * const _secondary_disp_z;
  /// z-displacement on the primary face
  const ADVariableValue * const _primary_disp_z;

  /// This factor multiplies the weighted gap. This member, provided through a user parameter,
  /// should be of a value such that its product with the gap is on the same scale as the lagrange
  /// multiplier
  const Real _c;

  /// The value of the gap at the current quadrature point
  ADReal _qp_gap;

  /// The value of the LM at the current quadrature point
  Real _qp_factor;

  /// Whether to normalize weighted gap by weighting function norm
  bool _normalize_c;

  /// Whether the dof objects are nodal; if they're not, then they're elemental
  const bool _nodal;

  /// The x displacement variable
  const MooseVariable * const _disp_x_var;
  /// The y displacement variable
  const MooseVariable * const _disp_y_var;
  /// The z displacement variable
  const MooseVariable * const _disp_z_var;

  /// Vector for computation of weighted gap with nodal normals
  ADRealVectorValue _qp_gap_nodal;

  /// A map from node to weighted gap and normalization (if requested)
  std::unordered_map<const DofObject *, std::pair<ADReal, Real>> _dof_to_weighted_gap;

  /// A pointer members that can be used to help avoid copying ADReals
  const ADReal * _weighted_gap_ptr = nullptr;
  const Real * _normalization_ptr = nullptr;

  /// A small threshold gap value to consider that a node needs a "persistency" constraint
  const Real _capture_tolerance;

  const ADVariableValue & _secondary_x_dot;
  const ADVariableValue & _primary_x_dot;
  const ADVariableValue & _secondary_y_dot;
  const ADVariableValue & _primary_y_dot;

  const ADVariableValue * _secondary_z_dot;
  const ADVariableValue * _primary_z_dot;

  /// Flag to determine whether wear needs to be included in the contact constraints
  const bool _has_wear;

  /// Wear depth to include contact
  const VariableValue & _wear_depth;

  /// A map from dof-object to the old weighted gap
  std::unordered_map<const DofObject *, ADReal> _dof_to_old_weighted_gap;

  /// Vector for computation of weighted gap velocity to fulfill "persistency" condition
  ADRealVectorValue _qp_gap_nodal_dynamics;

  /// Vector for computation of weighted gap velocity to fulfill "persistency" condition
  ADRealVectorValue _qp_velocity;

  /// A map from node to weighted gap velocity times _dt
  std::unordered_map<const DofObject *, ADReal> _dof_to_weighted_gap_dynamics;

  /// A map from node to weighted gap velocity times _dt
  std::unordered_map<const DofObject *, ADReal> _dof_to_velocity;
  /// A map from node to weighted gap velocity times _dt
  std::unordered_map<const DofObject *, ADReal> _dof_to_old_velocity;

  // Newmark-beta beta parameter
  const Real _newmark_beta;

  // Newmark-beta gamma parameter
  const Real _newmark_gamma;

  /// A map from node to wear in this step
  std::unordered_map<const DofObject *, ADReal> _dof_to_nodal_wear_depth;

  /// A map from node to wear in old step
  std::unordered_map<const DofObject *, ADReal> _dof_to_nodal_old_wear_depth;

  /// The relative velocity
  ADRealVectorValue _relative_velocity;
};
