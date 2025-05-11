//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CohesiveZoneModelBase.h"
#include "TwoVector.h"

/**
 * User object that interface pressure resulting from a simple traction separation law.
 */
class BilinearMixedModeCohesiveZoneModel : virtual public PenaltyWeightedGapUserObject,
                                           virtual public WeightedVelocitiesUserObject,
                                           virtual public CohesiveZoneModelBase
{
public:
  static InputParameters validParams();

  BilinearMixedModeCohesiveZoneModel(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void finalize() override;
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

  // @{
  // Compute CZM bilinear traction law.
  virtual void computeModeMixity(const Node * const node);
  virtual void computeCriticalDisplacementJump(const Node * const node);
  virtual void computeFinalDisplacementJump(const Node * const node);
  virtual void computeEffectiveDisplacementJump(const Node * const node);
  virtual void computeDamage(const Node * const node);
  // @}

  /// Encapsulate the CZM constitutive behavior.
  virtual void computeCZMTraction(const Node * const node) override;

  /// After full damage, set traction to zero
  const bool _apply_zero_traction_after_damage;

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
};
