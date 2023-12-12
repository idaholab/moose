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

class BilinearMixedModeCohesiveZoneModel;

class MortarGenericTraction : public ADMortarLagrangeConstraint
{
public:
  static InputParameters validParams();

  MortarGenericTraction(const InputParameters & parameters);

protected:
  ADReal computeQpResidual(Moose::MortarType type) final;

  /// Displacement component on which the residual will be computed
  const MooseEnum _component;

  /// The cohesive zone user object that provides the surface traction
  const BilinearMixedModeCohesiveZoneModel & _cohesize_zone_uo;
};
