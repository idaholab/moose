//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WeightedGapUserObject.h"
#include "AugmentedLagrangeInterface.h"

template <typename>
class MooseVariableFE;
class AugmentedLagrangianContactProblemInterface;

/**
 * User object for computing weighted gaps and contact pressure for penalty based
 * mortar constraints
 */
class PenaltyWeightedGapUserObject : virtual public WeightedGapUserObject,
                                     public AugmentedLagrangeInterface
{
public:
  static InputParameters validParams();

  PenaltyWeightedGapUserObject(const InputParameters & parameters);

  virtual void timestepSetup() override;

  virtual const ADVariableValue & contactPressure() const override;
  virtual void initialize() override;
  virtual void finalize() override;
  virtual void reinit() override;

  virtual Real getNormalContactPressure(const Node * const node) const override;
  virtual Real getNormalLagrangeMultiplier(const Node * const node) const;

  virtual Real getDeltaTangentialLagrangeMultiplier(const Node * const, const unsigned int) const
  {
    return 0.0;
  };
  virtual bool getActiveSetState(const Node * const node) const
  {
    return _dof_to_normal_pressure.find(_subproblem.mesh().nodePtr(node->id()))->second > 0;
  }

  virtual bool isAugmentedLagrangianConverged() override;
  virtual void augmentedLagrangianSetup() override;
  virtual void updateAugmentedLagrangianMultipliers() override;

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return false; }

  void selfInitialize();
  void selfFinalize();
  void selfTimestepSetup();

  /**
   * Adaptive, local penalty for AL. See 'The adapted augmented Lagrangian method: a new method for
   * the resolution of the mechanical frictional contact problem', Comput Mech (2012) 49: 259-275
   */
  void
  bussettaAdaptivePenalty(const Real previous_gap, const Real gap, Real & penalty, Real & eval_tn);

  /**
   * See Algorithm 3 of 'The adapted augmented Lagrangian method: a new method for
   * the resolution of the mechanical frictional contact problem', Comput Mech (2012) 49: 259-275
   */
  void adaptiveNormalPenalty(const Real previous_gap, const Real gap, Real & penalty);

  /// The penalty factor
  const Real _penalty;

  /// penalty growth factor for augmented Lagrange
  const Real _penalty_multiplier;

  /// penetration tolerance for augmented Lagrange contact
  const Real _penetration_tolerance;

  /// The contact pressure on the mortar segument quadrature points
  ADVariableValue _contact_pressure;

  /// Map from degree of freedom to normal pressure for reporting
  std::unordered_map<const DofObject *, ADReal> _dof_to_normal_pressure;

  ///@{ augmented Lagrange problem and iteration number
  AugmentedLagrangianContactProblemInterface * const _augmented_lagrange_problem;
  const static unsigned int _no_iterations;
  const unsigned int & _lagrangian_iteration_number;
  ///@}

  /// Map from degree of freedom to augmented lagrange multiplier
  std::unordered_map<const DofObject *, Real> _dof_to_lagrange_multiplier;

  /// Map from degree of freedom to local penalty value
  std::unordered_map<const DofObject *, Real> _dof_to_local_penalty;

  /// Map from degree of freedom to previous AL iteration gap values
  std::unordered_map<const DofObject *, Real> _dof_to_previous_gap;

  /// Current delta t... or timestep size.
  const Real & _dt;

  /// previous timestep size
  Real _dt_old;

  /// Use scaled or physical gap
  const bool _use_physical_gap;

  /// The auxiliary Lagrange multiplier variable (used together whith the Petrov-Galerkin approach)
  const MooseVariable * const _aux_lm_var;

  /// Maximum multiplier applied to the initial penalty factor in AL
  const Real _max_penalty_multiplier;

  /// The adaptivity method for the penalty factor at augmentations
  const enum class AdaptivityNormalPenalty { SIMPLE, BUSSETTA } _adaptivity_normal;
};
