//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarConstraint.h"

/**
 * Constrain the value of a variable to be the same on both sides of an
 * interface using a generalized force stemming from a penalty-based enforcement.
 */
class PenaltyEqualValueConstraint : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  PenaltyEqualValueConstraint(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType mortar_type) final;

  /// Penalty value used to enforce the constraint
  const Real _penalty_value;
};
