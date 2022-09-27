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

  /**
   * computes a single emissivity coefficient based on the coordinate system and the individual
   * secondary and primary surface emissivities
   */
  Real emissivity() const;

  /// Stefan-Boltzmann constant
  const Real _stefan_boltzmann;

  /// The index for the radial coordinate when performing RZ simulations
  unsigned int _radial_coord;

  /// The emissivity of the primary surface
  const Real _eps_primary;

  /// The emissivity of the secondary surface
  const Real _eps_secondary;

  /// Whether both primary and secondary emissivities are zero
  const bool _has_zero_emissivity;

  /// The theoretical infinite parallel-plate geometry surface emissivity, which is a function of
  /// both the primary and secondary surface emissivities
  const Real _parallel_plate_emissivity;

  /// The coordinate system type, e.g. XYZ or RZ
  Moose::CoordinateSystemType _coord_system;
};
