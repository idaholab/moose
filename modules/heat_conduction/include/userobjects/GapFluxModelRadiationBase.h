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
 * Gap flux model for heat conduction across a gap due to radiation, based on the diffusion
 * approximation. Provides the \p computeRadiationFlux routine which defines the radiation physics
 */
class GapFluxModelRadiationBase : public GapFluxModelBase
{
public:
  static InputParameters validParams();

  GapFluxModelRadiationBase(const InputParameters & parameters);

protected:
  /**
   * computes the radiation flux based on the input secondary and primary temperatures
   */
  ADReal computeRadiationFlux(const ADReal & secondary_T, const ADReal & primary_T) const;

  /// Stefan-Boltzmann constant
  const Real _stefan_boltzmann;

  /// The inverse of the surface emissivity
  Real _inverse_emissivity;
};
