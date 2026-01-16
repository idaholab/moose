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

  Moose::FV::GradientLimiterType gradientLimiter() const { return _gradient_limiter; }

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
  Moose::FV::GradientLimiterType _gradient_limiter =
      Moose::FV::GradientLimiterType::Venkatakrishnan;
  Real _deferred_correction_factor = 1.0;
};
