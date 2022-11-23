//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADMortarLagrangeConstraint.h"

/**
 * Applies mortar generalized forces from Lagrange multipliers defined in the global Cartesian frame
 * of reference.
 */
class CartesianMortarMechanicalContact : public ADMortarLagrangeConstraint
{
public:
  static InputParameters validParams();

  CartesianMortarMechanicalContact(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType type) final;

  /// The Cartesian component to compute the generalized force
  const MooseEnum _component;
};
