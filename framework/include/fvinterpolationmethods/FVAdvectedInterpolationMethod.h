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
#include "MooseFunctor.h"

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
   * Compute the matrix weights for the advected face value. Interpolation is used on
   * internal faces, boundary treatment is localized to the boundary conditions.
   * @param face The face being interpolated.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   * @param elem_grad Element-side cell gradient (required).
   * @param neighbor_grad Neighbor-side cell gradient (required).
   * @param mass_flux Face mass flux for determining upwind direction.
   */
  virtual AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                         Real elem_value,
                                                         Real neighbor_value,
                                                         const VectorValue<Real> * elem_grad,
                                                         const VectorValue<Real> * neighbor_grad,
                                                         Real mass_flux) const = 0;

  /**
   * Convenience overload that evaluates a scalar Moose functor at the adjacent cell centers and,
   * if needed, its cell gradients before applying this interpolation method. This will provide
   * contributions to the matrix and right hand side of a linear system.
   * @param functor The function which will be interpolated onto the face.
   * @param face The face which will be use for interpolation.
   * @param state The state argument for which we are performing the interpolation.
   * @param mass_flux The mass flux which will be used for the interpolation.
   */
  AdvectedSystemContribution advectedInterpolate(const Moose::FunctorBase<Real> & functor,
                                                 const FaceInfo & face,
                                                 const Moose::StateArg & state,
                                                 const Real mass_flux) const;

  /**
   * Compute the advected face value. Interpolation is used on
   * internal faces, boundary treatment is localized to the boundary conditions.
   * @param face The face being interpolated.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   * @param elem_grad Element-side cell gradient (required).
   * @param neighbor_grad Neighbor-side cell gradient (required).
   * @param mass_flux Face mass flux for determining upwind direction.
   */
  virtual Real advectedInterpolateValue(const FaceInfo & face,
                                        Real elem_value,
                                        Real neighbor_value,
                                        const VectorValue<Real> * elem_grad,
                                        const VectorValue<Real> * neighbor_grad,
                                        Real mass_flux) const;

  /**
   * Convenience overload that evaluates a scalar Moose functor at the adjacent cell centers and,
   * if needed, its cell gradients before applying this interpolation method. This will return
   * the interpolated face value.
   * @param functor The function which will be interpolated onto the face.
   * @param face The face which will be use for interpolation.
   * @param state The state argument for which we are performing the interpolation.
   * @param mass_flux The mass flux which will be used for the interpolation.
   */
  Real advectedInterpolateValue(const Moose::FunctorBase<Real> & functor,
                                const FaceInfo & face,
                                const Moose::StateArg & state,
                                const Real mass_flux) const;

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
