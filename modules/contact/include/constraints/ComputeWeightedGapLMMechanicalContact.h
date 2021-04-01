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
 * Computes the weighted gap that will later be used to enforce the
 * zero-penetration mechanical contact conditions
 */
class ComputeWeightedGapLMMechanicalContact : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  ComputeWeightedGapLMMechanicalContact(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType mortar_type) final;

  const ADVariableValue & _secondary_disp_x;
  const ADVariableValue & _primary_disp_x;
  const ADVariableValue & _secondary_disp_y;
  const ADVariableValue & _primary_disp_y;

  /// Whether this object is operating on the displaced mesh
  const bool _displaced;
};
