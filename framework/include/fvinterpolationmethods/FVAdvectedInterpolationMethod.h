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
   * Advected interpolation operation for this method.
   */
  virtual AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                         Real elem_value,
                                                         Real neighbor_value,
                                                         const VectorValue<Real> * elem_grad,
                                                         const VectorValue<Real> * neighbor_grad,
                                                         Real mass_flux) const = 0;

  /**
   * Convenience overload that evaluates a scalar Moose functor at the adjacent cell centers and,
   * if needed, its cell gradients before applying this interpolation method.
   */
  AdvectedSystemContribution advectedInterpolate(const Moose::FunctorBase<Real> & functor,
                                                 const FaceInfo & face,
                                                 const Moose::StateArg & state,
                                                 const Real mass_flux) const
  {
    mooseAssert(face.neighborPtr(),
                "Advected interpolation with a Moose functor requires a two-sided internal face.");

    const Moose::ElemArg elem_arg{face.elemPtr(), false};
    const Moose::ElemArg neighbor_arg{face.neighborPtr(), false};
    const Real elem_value = functor(elem_arg, state);
    const Real neighbor_value = functor(neighbor_arg, state);

    if (!needsGradients())
      return advectedInterpolate(face, elem_value, neighbor_value, nullptr, nullptr, mass_flux);

    const auto elem_grad = functor.gradient(elem_arg, state);
    const auto neighbor_grad = functor.gradient(neighbor_arg, state);
    return advectedInterpolate(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
  }

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
   * Convenience overload that evaluates a scalar Moose functor at the adjacent cell centers and
   * returns only the implied face value.
   */
  Real advectedInterpolateValue(const Moose::FunctorBase<Real> & functor,
                                const FaceInfo & face,
                                const Moose::StateArg & state,
                                const Real mass_flux) const
  {
    mooseAssert(face.neighborPtr(),
                "Advected interpolation with a Moose functor requires a two-sided internal face.");

    const Moose::ElemArg elem_arg{face.elemPtr(), false};
    const Moose::ElemArg neighbor_arg{face.neighborPtr(), false};
    const Real elem_value = functor(elem_arg, state);
    const Real neighbor_value = functor(neighbor_arg, state);

    if (!needsGradients())
      return advectedInterpolateValue(
          face, elem_value, neighbor_value, nullptr, nullptr, mass_flux);

    const auto elem_grad = functor.gradient(elem_arg, state);
    const auto neighbor_grad = functor.gradient(neighbor_arg, state);
    return advectedInterpolateValue(
        face, elem_value, neighbor_value, &elem_grad, &neighbor_grad, mass_flux);
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
