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
 * Base class for creating new nodally-based mortar user objects
 */
class PenaltyWeightedGapUserObject : public WeightedGapUserObject
{
public:
  static InputParameters validParams();

  PenaltyWeightedGapUserObject(const InputParameters & parameters);

  virtual const ADVariableValue & contactForce() const override;
  virtual void reinit(const Elem & lower_d_secondary_elem) override;
  virtual bool hasDof(const DofObject & dof_object) const override;

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool isWeightedGapNodal() const override;

  /// The penalty factor
  const Real _penalty;

  /// The contact force on the mortar segument quadrature points
  ADVariableValue _contact_force;
};
