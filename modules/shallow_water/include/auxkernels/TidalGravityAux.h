//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 * Computes a scalar gravity field g(t,x) including a simple global
 * Sun/Moon tidal correction projected onto the local vertical.
 *
 * Assumes the domain is a sphere centered at the origin; the local vertical is
 * the outward radial unit vector n_r = x/|x|. The effective gravity vector is
 * g_vec = -g0 * n_r + a_tide(t), and the scalar gravity for SWE is
 * g = -g_vec · n_r.
 *
 * Intended to be used with a MONOMIAL/CONSTANT AuxVariable so values are
 * element-wise constant and consistent on faces.
 */
class TidalGravityAux : public AuxKernel
{
public:
  static InputParameters validParams();

  TidalGravityAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;
  void updateTidal();

  // Parameters
  const Real _g0;
  const bool _enable_tides;
  const Real _earth_radius;
  const Real _t0_epoch;

  const Real _mu_sun;
  const Real _sun_distance;
  const Real _sun_period;

  const Real _mu_moon;
  const Real _moon_distance;
  const Real _moon_period;

  // Cache
  unsigned int _cached_t_step;
  RealVectorValue _a_tide;
};

