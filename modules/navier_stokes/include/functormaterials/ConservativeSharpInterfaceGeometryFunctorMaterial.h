//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FunctorMaterial.h"

/**
 * Functor material that prepares face-aware sharp-interface geometry quantities
 * for reduced-pressure linear-FV coupling.
 *
 * This object is intentionally focused on the *consumer-facing* quantities that
 * the custom Rhie-Chow object needs right now:
 *
 * - clipped / near-interface alpha indicators
 * - face unit normals from grad(alpha)
 */
class ConservativeSharpInterfaceGeometryFunctorMaterial final : public FunctorMaterial
{
public:
  static InputParameters validParams();

  ConservativeSharpInterfaceGeometryFunctorMaterial(const InputParameters & parameters);

private:
  template <typename SpaceArg>
  RealVectorValue alphaGradient(const SpaceArg & r, const Moose::StateArg & t) const;
  RealVectorValue alphaGradient(const Moose::FaceArg & r, const Moose::StateArg & t) const;

  const Moose::Functor<Real> & _volume_fraction;

  const bool _clip_volume_fraction;
  const Real _alpha_lower_bound;
  const Real _alpha_upper_bound;
  const Real _near_interface_lower;
  const Real _near_interface_upper;
  const Real _delta_n;

  const MooseFunctorName _delta_n_name;
  const MooseFunctorName _near_interface_name;
  const MooseFunctorName _alpha_gradient_name;
  const MooseFunctorName _interface_unit_normal_name;
};
