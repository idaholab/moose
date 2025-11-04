//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "WeightedVelocitiesUserObject.h"
#include "LMWeightedGapUserObject.h"

template <typename>
class MooseVariableFE;

/**
 * Nodal-based mortar contact user object for frictional problem
 */
class LMWeightedVelocitiesUserObject : public WeightedVelocitiesUserObject,
                                       public LMWeightedGapUserObject
{
public:
  static InputParameters validParams();

  LMWeightedVelocitiesUserObject(const InputParameters & parameters);

  virtual const ADVariableValue & contactTangentialPressureDirOne() const override;
  virtual const ADVariableValue & contactTangentialPressureDirTwo() const override;

protected:
  /// The Lagrange multiplier variables representing the tangential contact pressure
  const MooseVariableFE<Real> * const _lm_variable_tangential_one;
  const MooseVariableFE<Real> * const _lm_variable_tangential_two;
};
