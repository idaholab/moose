//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PenaltyWeightedGapUserObject.h"
#include "WeightedVelocitiesUserObject.h"
#include "AugmentedLagrangeInterface.h"
#include "TwoVector.h"

/**
 * User object that interface pressure resulting from a simple traction separation law.
 */
class PenaltySimpleCohesiveZoneModel : virtual public PenaltyWeightedGapUserObject,
                                       virtual public WeightedVelocitiesUserObject
{
public:
  static InputParameters validParams();

  PenaltySimpleCohesiveZoneModel(const InputParameters & parameters);

  virtual const ADVariableValue & contactTangentialPressureDirOne() const override;
  virtual const ADVariableValue & contactTangentialPressureDirTwo() const override;

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void reinit() override;
  virtual void timestepSetup() override;

  // Check that we are *not* doing AL
  // virtual bool isAugmentedLagrangianConverged() override;

protected:
  virtual void computeQpPropertiesLocal() override;
  virtual void computeQpIPropertiesLocal() override;

  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return false; }
  virtual void applyTractionSeparationLaw(const Node * const node);
  virtual void prepareJumpKinematicQuantities();
  virtual ADRankTwoTensor normalizeRankTwoTensorQuantity(const std::unordered_map<const DofObject *, ADRankTwoTensor> & map, const Node * const node);

  /// The normal penalty factor
  const Real _penalty;

  /// The penalty factor for the frictional constraints
  const Real _penalty_friction;

  /// Acceptable slip distance for augmented Lagrange convergence
  const Real _slip_tolerance;

  /// The friction coefficient
  const Real _friction_coefficient;

  /// Map from degree of freedom to current and old step slip
  std::unordered_map<const DofObject *, std::pair<TwoVector, TwoVector>> _dof_to_step_slip;

  /// Map from degree of freedom to current and old accumulated slip
  std::unordered_map<const DofObject *, std::pair<TwoVector, TwoVector>> _dof_to_accumulated_slip;

  /// Map from degree of freedom to current and old tangential traction
  std::unordered_map<const DofObject *, std::pair<ADTwoVector, TwoVector>>
      _dof_to_tangential_traction;

  /// Map from degree of freedom to czm normal traction
  std::unordered_map<const DofObject *, ADReal> _dof_to_czm_normal_traction;

  /// The first frictional contact pressure on the mortar segment quadrature points
  ADVariableValue _frictional_contact_traction_one;

  /// The second frictional contact pressure on the mortar segment quadrature points
  ADVariableValue _frictional_contact_traction_two;

  /// Map from degree of freedom to augmented lagrange multiplier
  std::unordered_map<const DofObject *, TwoVector> _dof_to_frictional_lagrange_multipliers;

  /// Map from degree of freedom to local friction penalty value
  std::unordered_map<const DofObject *, Real> _dof_to_local_penalty_friction;

  /// Penalty growth factor for augmented Lagrange
  const Real _penalty_multiplier_friction;

  /// The adaptivity method for the penalty factor at augmentations
  const enum class AdaptivityFrictionalPenalty { SIMPLE, FRICTION_LIMIT } _adaptivity_friction;

  /// Penalty for normal cohesive zone model
  const Real _czm_normal_stiffness;

  /// Penalty for tangential cohesive zone model
  const Real _czm_tangential_stiffness;

  /// Strength for normal cohesive zone model
  const Real _czm_normal_strength;

  /// Strength for tangential cohesive zone model
  const Real _czm_tangential_strength;

  /// The stress tensor on the interface
  // const ADMaterialProperty<RankTwoTensor> & _stress;

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
};
