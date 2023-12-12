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
  virtual const ADVariableValue & czmGlobalTraction(unsigned int i) const;

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

  // Compute CZM kinematics.
  virtual void prepareJumpKinematicQuantities();
  virtual void computeFandR(const Node * const node);

  // @{
  // Compute CZM bilinear traction law.
  virtual void computeModeMixity(const Node * const node);
  virtual void computeCriticalDisplacementJump(const Node * const node);
  virtual void computeFinalDisplacementJump(const Node * const node);
  virtual void computeEffectiveDisplacementJump(const Node * const node);
  virtual void computeDamage(const Node * const node);
  // @}

  /// Encapsulate the CZM constitutive behavior.
  virtual void computeBilinearMixedModeTraction(const Node * const node);

  /// Compute global traction for mortar application
  virtual void computeGlobalTraction(const Node * const node);

  /// Normalize rank two tensor mortar quantities (remove mortar integral scaling)
  virtual ADRankTwoTensor
  normalizeRankTwoTensorQuantity(const std::unordered_map<const DofObject *, ADRankTwoTensor> & map,
                                 const Node * const node);

  /// Normalize real mortar quantities (remove mortar integral scaling)
  virtual ADReal normalizeRealQuantity(const std::unordered_map<const DofObject *, ADReal> & map,
                                       const Node * const node);

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

  /// Whether to use the bilinear mixed mode traction model
  const bool _use_bilinear_mixed_mode_traction;

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

  /// Map from degree of freedom to mode mixity ratio (AD needed?)
  std::unordered_map<const DofObject *, ADReal> _dof_to_mode_mixity_ratio;

  /// The normal strength material property
  const MaterialProperty<Real> & _normal_strength;

  /// The shear strength material property
  const MaterialProperty<Real> & _shear_strength;

  /// Fracture parameter mode I
  const MaterialProperty<Real> & _GI_c;

  /// Fracture parameter mode II
  const MaterialProperty<Real> & _GII_c;

  /// Interpolated value of normal_strength
  ADReal _normal_strength_interpolation;

  /// Interpolated value of shear_strength
  ADReal _shear_strength_interpolation;

  /// Interpolated value of fracture paramter mode I
  ADReal _GI_c_interpolation;

  /// Interpolated value of fracture paramter mode II
  ADReal _GII_c_interpolation;

  // Penalty stiffness for bilinear traction model
  const Real _penalty_stiffness_czm;

  /// Mixed-mode propagation criterion
  enum class MixedModeCriterion
  {
    POWER_LAW,
    BK
  } _mix_mode_criterion;

  /// Power law parameter for bilinear traction model
  const Real _power_law_parameter;

  /// Viscosity for damage model
  const Real _viscosity;

  /// Parameter for the regularization of the Macaulay bracket
  const Real _regularization_alpha;

  /// Tolerance to avoid NaN/Inf in automatic differentiation operations.
  const Real _epsilon_tolerance;

  /// The global traction
  ADVariableValue _czm_interpolated_traction_x;
  ADVariableValue _czm_interpolated_traction_y;
  ADVariableValue _czm_interpolated_traction_z;

  // @{
  // Strength material properties at the nodes
  std::unordered_map<const DofObject *, ADReal> _dof_to_normal_strength;
  std::unordered_map<const DofObject *, ADReal> _dof_to_shear_strength;
  // @}

  // @{
  // Fracture properties at the nodes
  std::unordered_map<const DofObject *, ADReal> _dof_to_GI_c;
  std::unordered_map<const DofObject *, ADReal> _dof_to_GII_c;
  // @}

  // @{
  // The parameters in the damage evolution law: Maps node to damage
  std::unordered_map<const DofObject *, ADReal> _dof_to_delta_initial;
  std::unordered_map<const DofObject *, ADReal> _dof_to_delta_final;
  std::unordered_map<const DofObject *, ADReal> _dof_to_delta_medium;
  std::unordered_map<const DofObject *, std::pair<ADReal, Real>> _dof_to_damage;
  // @}

  /// Total Lagrangian stress to be applied on CZM interface
  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_czm_traction;
};
