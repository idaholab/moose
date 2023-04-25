//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WeightedVelocitiesUserObject.h"

/**
 * User object that computes tangential pressures due to friction using a penalty approach
 */
class PenaltyFrictionUserObject : public WeightedVelocitiesUserObject
{
public:
  static InputParameters validParams();

  PenaltyFrictionUserObject(const InputParameters & parameters);

  virtual const ADVariableValue & contactPressure() const override;
  virtual const ADVariableValue & contactTangentialPressureDirOne() const override;
  virtual const ADVariableValue & contactTangentialPressureDirTwo() const override;

  virtual void initialize() override;
  virtual void finalize() override;
  virtual void reinit() override;
  virtual void timestepSetup() override;

  virtual Real getNormalContactPressure(const Node * const node) const override;

  virtual Real getFrictionalContactPressure(const Node * const node,
                                            const unsigned int component) const override;
  virtual Real getAccumulatedSlip(const Node * const node,
                                  const unsigned int component) const override;
  virtual Real getTangentialVelocity(const Node * const node,
                                     const unsigned int component) const override;

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return false; }

  /// The normal penalty factor
  const Real _penalty;

  /// The penalty factor for the frictional constraints
  const Real _penalty_friction;

  /// The friction coefficient
  const Real _friction_coefficient;

  /// Map from degree of freedom to old weighted gap
  std::unordered_map<const DofObject *, ADReal> _dof_to_old_weighted_gap;

  /// Map from degree of freedom to old physical (not weighted) tangential velocity
  std::unordered_map<const DofObject *, std::array<ADReal, 2>> _dof_to_old_real_tangential_velocity;

  /// Map from degree of freedom to accumulated slip
  std::unordered_map<const DofObject *, std::array<ADReal, 2>> _dof_to_accumulated_slip;

  /// Map from degree of freedom to old accumulated slip
  std::unordered_map<const DofObject *, std::array<ADReal, 2>> _dof_to_old_accumulated_slip;

  /// Map from degree of freedom to normal pressure
  std::unordered_map<const DofObject *, ADReal> _dof_to_normal_pressure;

  /// Map from degree of freedom to old normal pressure
  std::unordered_map<const DofObject *, Real> _dof_to_old_normal_pressure;

  /// Map from degree of freedom to both frictional (tangential) pressure direction
  std::unordered_map<const DofObject *, std::array<ADReal, 2>> _dof_to_frictional_pressure;

  /// The normal contact pressure on the mortar segument quadrature points
  ADVariableValue _contact_force;

  /// The first frictional contact pressure on the mortar segment quadrature points
  ADVariableValue _frictional_contact_force_one;

  /// The second frictional contact pressure on the mortar segment quadrature points
  ADVariableValue _frictional_contact_force_two;
};
