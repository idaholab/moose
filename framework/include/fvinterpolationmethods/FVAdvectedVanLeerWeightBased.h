//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVInterpolationMethod.h"
#include "Limiter.h"

#include <memory>

/**
 * Van Leer interpolation for advected quantities that blends between upwind and the
 * higher-order limited value using only (elem, neighbor) weights.
 *
 * This is implemented as a limited-scheme style blending weight (OpenFOAM-style) rather than a
 * MUSCL face reconstruction with deferred correction. Instead, it returns matrix weights that
 * incorporate the limiter directly.
 *
 * Note: The limiter coefficient depends on solution gradients, so this is typically used in a
 * fixed-point outer loop where the weights are lagged.
 */
class FVAdvectedVanLeerWeightBased : public FVInterpolationMethod
{
public:
  static InputParameters validParams();

  FVAdvectedVanLeerWeightBased(const InputParameters & params);

  AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                 Real elem_value,
                                                 Real neighbor_value,
                                                 const VectorValue<Real> * elem_grad,
                                                 const VectorValue<Real> * neighbor_grad,
                                                 Real mass_flux) const;

  Real advectedInterpolateValue(const FaceInfo & face,
                                Real elem_value,
                                Real neighbor_value,
                                const VectorValue<Real> * elem_grad,
                                const VectorValue<Real> * neighbor_grad,
                                Real mass_flux) const;

private:
  const std::unique_ptr<Moose::FV::Limiter<Real>> _limiter;
  const bool _limit_to_linear;
  const Real _blending_factor;
};
