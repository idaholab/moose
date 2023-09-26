//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GapFluxModelRadiationBase.h"

/**
 * Gap flux model for heat conduction across a gap due to radiation, based on the diffusion
 * approximation. Uses a coupled temperature variable to provide the arguments to the \p
 * computeRadiationFlux method in the base class
 */
class GapFluxModelRadiation : public GapFluxModelRadiationBase
{
public:
  static InputParameters validParams();

  GapFluxModelRadiation(const InputParameters & parameters);

  ADReal computeFlux() const override;

protected:
  /// Primary surface temperature
  const ADVariableValue & _primary_T;
  /// Secondary surface temperature
  const ADVariableValue & _secondary_T;
};
