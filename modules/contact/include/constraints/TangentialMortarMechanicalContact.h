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

class WeightedVelocitiesUserObject;

class TangentialMortarMechanicalContact : public ADMortarLagrangeConstraint
{
public:
  static InputParameters validParams();

  TangentialMortarMechanicalContact(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType type) final;

  /// Displacement component on which the residual will be computed
  const MooseEnum _component;

  /// Tangent direction used for computing the residual. In three-dimensions,
  /// there will be two tangent vectors.
  const MooseEnum _direction;

  /// The weighted velocities user object which supplies the contact pressure tangential vectors
  const WeightedVelocitiesUserObject & _weighted_velocities_uo;
};
