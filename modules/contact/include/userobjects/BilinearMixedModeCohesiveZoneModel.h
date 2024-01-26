//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "PenaltySimpleCohesiveZoneModel.h"
#include "TwoVector.h"

/**
 * User object that interface pressure resulting from a simple traction separation law.
 */
class BilinearMixedModeCohesiveZoneModel : virtual public PenaltyWeightedGapUserObject,
                                           virtual public WeightedVelocitiesUserObject,
                                           virtual public PenaltySimpleCohesiveZoneModel
{
public:
  static InputParameters validParams();

  BilinearMixedModeCohesiveZoneModel(const InputParameters & parameters);

  virtual const ADVariableValue & czmGlobalTraction(unsigned int i) const;

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void reinit() override;
  virtual void timestepSetup() override;

  // Getters for analysis output
  Real getModeMixityRatio(const Node * const node) const;
  Real getCohesiveDamage(const Node * const node) const;
  Real getLocalDisplacementNormal(const Node * const node) const;
  Real getLocalDisplacementTangential(const Node * const node) const;

protected:
  virtual void computeQpProperties() override;
  virtual void computeQpIProperties() override;

  virtual bool constrainedByOwner() const override { return false; }

  // Compute CZM kinematics.
  virtual void prepareJumpKinematicQuantities() override;
  virtual void computeFandR(const Node * const node) override;

  // @{
  // Compute CZM bilinear traction law.
  virtual void computeModeMixity(const Node * const node);
  virtual void computeCriticalDisplacementJump(const Node * const node);
  virtual void computeFinalDisplacementJump(const Node * const node);
  virtual void computeEffectiveDisplacementJump(const Node * const node);
  virtual void computeDamage(const Node * const node);
  // @}

  /// Encapsulate the CZM constitutive behavior.
  virtual void computeBilinearMixedModeTraction(const Node * const node) override;

  /// Compute global traction for mortar application
  virtual void computeGlobalTraction(const Node * const node) override;

  /// Normalize mortar quantities (remove mortar integral scaling)
  template <class T>
  T normalizeQuantity(const std::unordered_map<const DofObject *, T> & map,
                      const Node * const node);

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

  /// The global traction
  std::vector<ADVariableValue> _czm_interpolated_traction;

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
  std::unordered_map<const DofObject *, ADReal> _dof_to_delta_max;
  std::unordered_map<const DofObject *, std::pair<ADReal, Real>> _dof_to_damage;
  // @}

  /// Total Lagrangian stress to be applied on CZM interface
  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_czm_traction;

  std::unordered_map<const DofObject *, ADRealVectorValue> _dof_to_displacement_jump;
};
