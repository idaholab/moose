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
 * interface.
 */
class EqualValueConstraint : public ADMortarConstraint
{
public:
  static InputParameters validParams();

  EqualValueConstraint(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType mortar_type) final;

  /// The secondary face lower dimensional element (not the mortar element!) volume
  const Real & _lower_secondary_volume;

  /// The primary face lower dimensional element volume (not the mortar element!)
  const Real & _lower_primary_volume;

  /// The stabilization parameter
  const Real _delta;

  /// The diffusion coefficient on the secondary side
  const ADMaterialProperty<Real> & _diff_secondary;

  /// The diffusion coefficient on the primary side
  const ADMaterialProperty<Real> & _diff_primary;

  /// whether to perform stabilization. We have this parameter to save computational cost in the
  /// unstabilized case
  const bool _stabilize;
};
