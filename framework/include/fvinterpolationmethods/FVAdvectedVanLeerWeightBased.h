//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVAdvectedInterpolationMethod.h"
#include "FVInterpolationMethod.h"

/**
 * Van Leer interpolation for advected quantities that blends between upwind and the
 * higher-order limited value using only (elem, neighbor) weights.
 */
class FVAdvectedVanLeerWeightBased : public FVInterpolationMethod,
                                     public FVAdvectedInterpolationMethod
{
public:
  static InputParameters validParams();

  FVAdvectedVanLeerWeightBased(const InputParameters & params);

  using FVAdvectedInterpolationMethod::advectedInterpolate;
  using FVAdvectedInterpolationMethod::advectedInterpolateValue;

  bool needsGradients() const override { return true; }

  AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                 Real elem_value,
                                                 Real neighbor_value,
                                                 const VectorValue<Real> * elem_grad,
                                                 const VectorValue<Real> * neighbor_grad,
                                                 Real mass_flux) const override;

  Real advectedInterpolateValue(const FaceInfo & face,
                                Real elem_value,
                                Real neighbor_value,
                                const VectorValue<Real> * elem_grad,
                                const VectorValue<Real> * neighbor_grad,
                                Real mass_flux) const override;

private:
  /// Whether to clamp the blending to be no more downwind-biased than linear.
  const bool _limit_to_linear;
  /// Scales the high-order blending strength (0 = upwind, 1 = full limited blending).
  const Real _blending_factor;
};
