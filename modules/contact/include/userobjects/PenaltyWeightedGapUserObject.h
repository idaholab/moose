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

template <typename>
class MooseVariableFE;

/**
 * User object for computing weighted gaps and contact pressure for penalty based
 * mortar constraints
 */
class PenaltyWeightedGapUserObject : public WeightedGapUserObject
{
public:
  static InputParameters validParams();

  PenaltyWeightedGapUserObject(const InputParameters & parameters);

  virtual const ADVariableValue & contactPressure() const override;
  virtual void initialize() override;
  virtual void reinit() override;
  virtual Real getNormalContactPressure(const Node * const node) const override;

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool constrainedByOwner() const override { return false; }

  /// The penalty factor
  const Real _penalty;

  /// The contact force on the mortar segument quadrature points
  ADVariableValue _contact_force;

  /// Map from degree of freedom to normal pressure for reporting
  std::unordered_map<const DofObject *, Real> _dof_to_normal_pressure;
};
