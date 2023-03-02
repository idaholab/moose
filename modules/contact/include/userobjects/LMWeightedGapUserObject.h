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
class LMWeightedGapUserObject : public WeightedGapUserObject
{
public:
  static InputParameters validParams();

  LMWeightedGapUserObject(const InputParameters & parameters);

protected:
  virtual const VariableTestValue & test() const override;
  virtual bool isWeightedGapNodal() const override;

  /// The Lagrange multiplier variable representing the contact pressure
  const MooseVariableFE<Real> * const _lm_var;
};
