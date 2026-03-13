//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FaceInfo.h"
#include "GradientLimiterType.h"

#include <utility>

/**
 * Interface for interpolation methods that provide matrix and RHS contributions for advected face
 * values.
 */
class FVAdvectedInterpolationMethod
{
public:
  /// Matrix/RHS contribution for an advected face interpolation.
  struct AdvectedSystemContribution
  {
    std::pair<Real, Real> weights_matrix;
    Real rhs_face_value = 0.0;
  };

  /**
   * Advected interpolation operation for this method.
   */
  virtual AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                         Real elem_value,
                                                         Real neighbor_value,
                                                         const VectorValue<Real> * elem_grad,
                                                         const VectorValue<Real> * neighbor_grad,
                                                         Real mass_flux) const = 0;

  /**
   * Advected interpolation operation that returns only the face value.
   */
  virtual Real advectedInterpolateValue(const FaceInfo & face,
                                        Real elem_value,
                                        Real neighbor_value,
                                        const VectorValue<Real> * elem_grad,
                                        const VectorValue<Real> * neighbor_grad,
                                        Real mass_flux) const
  {
    const auto result =
        advectedInterpolate(face, elem_value, neighbor_value, elem_grad, neighbor_grad, mass_flux);
    return result.weights_matrix.first * elem_value +
           result.weights_matrix.second * neighbor_value - result.rhs_face_value;
  }

  /**
   * Whether advected interpolation requires adjacent-cell gradients.
   */
  virtual bool needsGradients() const { return false; }

  /**
   * Limiter used by interpolations that require limited gradients.
   */
  virtual Moose::FV::GradientLimiterType gradientLimiter() const
  {
    return Moose::FV::GradientLimiterType::None;
  }
};
