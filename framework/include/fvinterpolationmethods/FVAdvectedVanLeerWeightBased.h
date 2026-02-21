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

/**
 * Van Leer interpolation for advected quantities that blends between upwind and the
 * higher-order limited value using only (elem, neighbor) weights.
 *
 * This is implemented as a limited-scheme style blending weight rather than a
 * MUSCL face reconstruction with deferred correction. Instead, it returns matrix weights that
 * incorporate the limiter directly.
 */
class FVAdvectedVanLeerWeightBased : public FVInterpolationMethod
{
public:
  static InputParameters validParams();

  FVAdvectedVanLeerWeightBased(const InputParameters & params);

  struct DeviceData
  {
    bool limit_to_linear = true;
    Real blending_factor = 1.0;
  };

  static AdvectedSystemContribution advectedInterpolate(const DeviceData & data,
                                                        const FaceInfo & face,
                                                        Real elem_value,
                                                        Real neighbor_value,
                                                        const VectorValue<Real> * elem_grad,
                                                        const VectorValue<Real> * neighbor_grad,
                                                        Real mass_flux);

  static Real advectedInterpolateValue(const DeviceData & data,
                                       const FaceInfo & face,
                                       Real elem_value,
                                       Real neighbor_value,
                                       const VectorValue<Real> * elem_grad,
                                       const VectorValue<Real> * neighbor_grad,
                                       Real mass_flux);

  /**
   * Compute the matrix weights for the advected face value.
   * @param face The face being interpolated.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   * @param elem_grad Element-side cell gradient (required).
   * @param neighbor_grad Neighbor-side cell gradient (required).
   * @param mass_flux Face mass flux; sign determines upwind direction.
   */
  AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                 Real elem_value,
                                                 Real neighbor_value,
                                                 const VectorValue<Real> * elem_grad,
                                                 const VectorValue<Real> * neighbor_grad,
                                                 Real mass_flux) const;

  /**
   * Convenience wrapper that returns only the face value implied by the weights.
   * @param face The face being interpolated.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   * @param elem_grad Element-side cell gradient (required).
   * @param neighbor_grad Neighbor-side cell gradient (required).
   * @param mass_flux Face mass flux; sign determines upwind direction.
   * @return The advected face value.
   */
  Real advectedInterpolateValue(const FaceInfo & face,
                                Real elem_value,
                                Real neighbor_value,
                                const VectorValue<Real> * elem_grad,
                                const VectorValue<Real> * neighbor_grad,
                                Real mass_flux) const;

private:
  /// Whether to clamp the blending to be no more downwind-biased than linear.
  const bool _limit_to_linear;
  /// Scales the high-order blending strength (0 = upwind, 1 = full limited blending).
  const Real _blending_factor;
};
