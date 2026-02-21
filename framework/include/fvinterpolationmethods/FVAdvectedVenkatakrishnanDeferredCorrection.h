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
 * Multi-dimensional MUSCL reconstruction using Venkatakrishnan-limited cell gradients and
 * deferred correction.
 *
 * The matrix contribution is pure upwind; the high-order reconstruction is added to the right hand
 * side as a deferred correction.
 */
class FVAdvectedVenkatakrishnanDeferredCorrection : public FVInterpolationMethod
{
public:
  static InputParameters validParams();

  FVAdvectedVenkatakrishnanDeferredCorrection(const InputParameters & params);

  struct DeviceData
  {
    Real deferred_correction_factor = 1.0;
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
   * @return The cell-gradient limiter used by this interpolation method.
   */
  Moose::FV::GradientLimiterType gradientLimiter() const { return _gradient_limiter; }

  /**
   * Compute the matrix weights and RHS deferred correction for the advected face value.
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
   * Convenience wrapper that returns the face value implied by matrix weights plus RHS correction.
   * @param face The face being interpolated.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   * @param elem_grad Element-side cell gradient (required).
   * @param neighbor_grad Neighbor-side cell gradient (required).
   * @param mass_flux Face mass flux; sign determines upwind direction.
   */
  Real advectedInterpolateValue(const FaceInfo & face,
                                Real elem_value,
                                Real neighbor_value,
                                const VectorValue<Real> * elem_grad,
                                const VectorValue<Real> * neighbor_grad,
                                Real mass_flux) const;

private:
  Moose::FV::GradientLimiterType _gradient_limiter =
      Moose::FV::GradientLimiterType::Venkatakrishnan;
  /// Scales the deferred correction strength (0 = upwind, 1 = full deferred correction).
  Real _deferred_correction_factor = 1.0;
};
