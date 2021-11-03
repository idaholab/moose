//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelBase.h"

/**
 * Base class for gap flux models used by ModularGapConductanceConstraint
 */
class GapFluxModelRadiative : public GapFluxModelBase
{
public:
  static InputParameters validParams();

  GapFluxModelRadiative(const InputParameters & parameters);

  ADReal computeFlux() const override;

protected:
  /// Primary surface temperature
  const ADVariableValue & _primary_T;
  /// Secondary surface temperature
  const ADVariableValue & _secondary_T;

  /// Stefan-Boltzmann constant
  const Real _sigma;

  /// Primary surface emissivity
  const ADMaterialProperty<Real> & _primary_emissivity;
  /// Secondary surface emissivity
  const ADMaterialProperty<Real> & _secondary_emissivity;
};
