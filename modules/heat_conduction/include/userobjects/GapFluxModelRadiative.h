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

  ADReal computeFlux(const ADReal & gap_width, unsigned int qp) const override;

protected:
  /// Thermal conductivity of the gap medium (e.g. air).
  const Real _k;

  /// Minimum gap distance allowed
  const Real _min_gap;

  /// Primary surface temperature
  const VariableValue & _primary_T;
  /// Secondary surface temperature
  const VariableValue & _secondary_T;

  /// Stefan-Boltzmann constant
  const Real _sigma;

  /// Primary surface emissivity
  const MaterialProperty<Real> & _primary_emissivity;
  /// Secondary surface emissivity
  const MaterialProperty<Real> & _secondary_emissivity;
};
