//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TwoVector.h"
#include "PenaltyWeightedGapUserObject.h"
#include "WeightedVelocitiesUserObject.h"
#include "AugmentedLagrangeInterface.h"

/**
 * User object that interface pressure resulting from a simple traction separation law.
 */
class CohesiveZoneModelBase : virtual public PenaltyWeightedGapUserObject,
                              virtual public WeightedVelocitiesUserObject
{
public:
  static InputParameters validParams();

  CohesiveZoneModelBase(const InputParameters & parameters);

  virtual const ADVariableValue & contactTangentialPressureDirOne() const override;
  virtual const ADVariableValue & contactTangentialPressureDirTwo() const override;

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void reinit() override;
  virtual void timestepSetup() override;
  virtual const ADVariableValue & czmGlobalTraction(unsigned int i) const;

protected:
  virtual void computeQpProperties() override;
  virtual void computeQpIProperties() override;

  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return false; }

  // Compute CZM kinematics.
  virtual void prepareJumpKinematicQuantities();
  virtual void computeFandR(const Node * const node);

  /// Encapsulate the CZM constitutive behavior.
  virtual void computeCZMTraction(const Node * const /*node*/) = 0;

  virtual void computeDamage(const Node * const /*node*/) = 0;

  /// Compute global traction for mortar application.
  virtual void computeGlobalTraction(const Node * const node);

  /// Normalize mortar quantities (remove mortar integral scaling)
  template <class T>
  T normalizeQuantity(const std::unordered_map<const DofObject *, T> & map, const Node * const node)
  {
    return libmesh_map_find(map, node) / _dof_to_weighted_gap[node].second;
  }

  /// Number of displacement components
  const unsigned int _ndisp;

  /// Coupled displacement gradients
  std::vector<const GenericVariableGradient<true> *> _grad_disp;

  /// Coupled displacement and neighbor displacement gradient
  std::vector<const GenericVariableGradient<true> *> _grad_disp_neighbor;

  /// *** Kinematics/displacement jump quantities ***
  /// Map from degree of freedom to rotation matrix
  std::unordered_map<const DofObject *, ADRankTwoTensor> _dof_to_rotation_matrix;

  /// Map from degree of freedom to local displacement jump
  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_interface_displacement_jump;

  /// Deformation gradient for interpolation
  ADRankTwoTensor _F_interpolation;

  /// Deformation gradient for interpolation of the neighbor projection
  ADRankTwoTensor _F_neighbor_interpolation;

  /// Map from degree of freedom to secondary, interpolated deformation gradient tensor
  std::unordered_map<const DofObject *, ADRankTwoTensor> _dof_to_F;

  /// Map from degree of freedom to neighbor, interpolated deformation gradient tensor
  std::unordered_map<const DofObject *, ADRankTwoTensor> _dof_to_F_neighbor;

  /// Map from degree of freedom to interface deformation gradient tensor
  std::unordered_map<const DofObject *, ADRankTwoTensor> _dof_to_interface_F;

  /// Map from degree of freedom to interface rotation tensor
  std::unordered_map<const DofObject *, ADRankTwoTensor> _dof_to_interface_R;

  /// The global traction
  std::vector<ADVariableValue> _czm_interpolated_traction;

  /// The penalty factor for the frictional constraints
  const Real _penalty_friction;

  /// The friction coefficient
  const Real _friction_coefficient;

  /// Map from degree of freedom to current and old step slip
  std::unordered_map<const DofObject *, std::pair<TwoVector, TwoVector>> _dof_to_step_slip;

  /// Map from degree of freedom to current and old accumulated slip
  std::unordered_map<const DofObject *, std::pair<TwoVector, TwoVector>> _dof_to_accumulated_slip;

  // Map from degree of freedom to current and old tangential traction
  std::unordered_map<const DofObject *, std::pair<ADTwoVector, TwoVector>>
      _dof_to_tangential_traction;

  // /// Map from degree of freedom to czm normal traction
  // std::unordered_map<const DofObject *, ADReal> _dof_to_czm_normal_traction;

  /// The first frictional contact pressure on the mortar segment quadrature points
  ADVariableValue _frictional_contact_traction_one;

  /// The second frictional contact pressure on the mortar segment quadrature points
  ADVariableValue _frictional_contact_traction_two;

  /// Map from degree of freedom to augmented lagrange multiplier
  std::unordered_map<const DofObject *, TwoVector> _dof_to_frictional_lagrange_multipliers;

  /// Map from degree of freedom to local friction penalty value
  std::unordered_map<const DofObject *, Real> _dof_to_local_penalty_friction;

  /// Tolerance to avoid NaN/Inf in automatic differentiation operations.
  const Real _epsilon_tolerance;

  /// Total Lagrangian stress to be applied on CZM interface
  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_czm_traction;

  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_displacement_jump;

  std::unordered_map<const DofObject *, std::pair<ADReal, Real>> _dof_to_damage;
};
