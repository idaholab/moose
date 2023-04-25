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
#include "PenaltyMortarAugmentedLagrangeInterface.h"

template <typename>
class MooseVariableFE;
class AugmentedLagrangianContactProblem;

/**
 * User object for computing weighted gaps and contact pressure for penalty based
 * mortar constraints
 */
class PenaltyWeightedGapUserObject : public WeightedGapUserObject,
                                     public PenaltyMortarAugmentedLagrangeInterface
{
public:
  static InputParameters validParams();

  PenaltyWeightedGapUserObject(const InputParameters & parameters);

  virtual void timestepSetup() override;

  virtual const ADVariableValue & contactPressure() const override;
  virtual void initialize() override;
  virtual void reinit() override;

  virtual Real getNormalContactPressure(const Node * const node) const override;

  virtual bool isContactConverged() override;
  virtual void updateAugmentedLagrangianMultipliers() override;

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return false; }

  /// The penalty factor
  const Real _penalty;

  /// penalty growth factor for augmented Lagrange
  const Real _penalty_multiplier;

  /// penetration tolerance for augmented Lagrange contact
  const Real _penetration_tolerance;

  /// The contact force on the mortar segument quadrature points
  ADVariableValue _contact_force;

  /// Map from degree of freedom to normal pressure for reporting
  std::unordered_map<const DofObject *, Real> _dof_to_normal_pressure;

  ///@{ augmented Lagrange probmen and iteration number
  AugmentedLagrangianContactProblem * const _augmented_lagrange_problem;
  const static unsigned int _no_iterations;
  const unsigned int & _lagrangian_iteration_number;
  ///@}

  /// Map from degree of freedom to augmented lagrange multiplier
  std::unordered_map<const DofObject *, Real> _dof_to_lagrange_multiplier;

  bool _new_time_step;
};
