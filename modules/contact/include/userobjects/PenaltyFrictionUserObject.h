//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
 * User object that computes tangential pressures due to friction using a penalty approach,
 * following J.C. Simo, T.A. Laursen, An augmented lagrangian treatment of contact problems
 * involving friction, https://doi.org/10.1016/0045-7949(92)90540-G.
 */
class PenaltyFrictionUserObject : virtual public PenaltyWeightedGapUserObject,
                                  virtual public WeightedVelocitiesUserObject
{
public:
  static InputParameters validParams();

  PenaltyFrictionUserObject(const InputParameters & parameters);

  virtual const ADVariableValue & contactTangentialPressureDirOne() const override;
  virtual const ADVariableValue & contactTangentialPressureDirTwo() const override;

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void reinit() override;
  virtual ADReal nodalTangentialPressure(const Node & node, unsigned int direction) const override;
  virtual void timestepSetup() override;

  virtual Real getFrictionalContactPressure(const Node * const node,
                                            const unsigned int component) const override;
  virtual Real getAccumulatedSlip(const Node * const node,
                                  const unsigned int component) const override;
  virtual Real getTangentialVelocity(const Node * const node,
                                     const unsigned int component) const override;
  virtual Real getDeltaTangentialLagrangeMultiplier(const Node * const node,
                                                    const unsigned int component) const override;

  virtual bool isAugmentedLagrangianConverged() override;
  virtual void updateAugmentedLagrangianMultipliers() override;

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return false; }

  /// The normal penalty factor
  const Real _penalty;

  /// The penalty factor for the frictional constraints
  const Real _penalty_friction;

  /// Acceptable slip distance for augmented Lagrange convergence
  const Real _slip_tolerance;

  /// The friction coefficient
  const Real _friction_coefficient;

  // The maps below are keyed by node id so that they can be stored as restartable data. Recovery
  // must restore their keys as well as their values: the augmented Lagrange updates loop over every
  // node a map holds, including nodes that were in contact only in earlier timesteps.

  /// Map from node id to current and old step slip
  std::unordered_map<dof_id_type, std::pair<TwoVector, TwoVector>> & _dof_to_step_slip;

  /// Map from node id to current and old accumulated slip
  std::unordered_map<dof_id_type, std::pair<TwoVector, TwoVector>> & _dof_to_accumulated_slip;

  /// Map from node id to current and old tangential traction
  std::unordered_map<dof_id_type, std::pair<ADTwoVector, TwoVector>> & _dof_to_tangential_traction;

  /// The first frictional contact pressure on the mortar segment quadrature points
  ADVariableValue _frictional_contact_traction_one;

  /// The second frictional contact pressure on the mortar segment quadrature points
  ADVariableValue _frictional_contact_traction_two;

  /// Map from node id to augmented lagrange multiplier
  std::unordered_map<dof_id_type, TwoVector> & _dof_to_frictional_lagrange_multipliers;

  /// Map from node id to local friction penalty value
  std::unordered_map<dof_id_type, Real> & _dof_to_local_penalty_friction;

  /// Penalty growth factor for augmented Lagrange
  const Real _penalty_multiplier_friction;

  /// The adaptivity method for the penalty factor at augmentations
  const enum class AdaptivityFrictionalPenalty { SIMPLE, FRICTION_LIMIT } _adaptivity_friction;

  /// Tolerance to avoid NaN/Inf in automatic differentiation operations.
  const Real _epsilon_tolerance;
};
