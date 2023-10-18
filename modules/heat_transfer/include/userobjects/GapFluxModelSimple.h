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
class GapFluxModelSimple : public GapFluxModelBase
{
public:
  static InputParameters validParams();

  GapFluxModelSimple(const InputParameters & parameters);

  ADReal computeFlux() const override;

protected:
  /// Thermal conductivity of the gap medium (e.g. air).
  const Real _k;

  /// Minimum gap distance allowed
  const Real _min_gap;

  /// Temperatures
  const ADVariableValue & _primary_T;
  const ADVariableValue & _secondary_T;
};
