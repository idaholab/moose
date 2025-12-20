//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "FaceInfo.h"
#include "MooseError.h"

/**
 * Base class for face interpolation functions used by linear FV objects.
 * Maybe we can extend it a little late to regular FV as well.
 *
 * These user objects expose lightweight callable interpolators that can be cached by kernels so
 * that interpolation does not require virtual dispatch inside hot loops. Derived classes only need
 * to implement the actual interpolation formula and register it through the helper methods provided
 * here.
 *
 * The logic is a little twisted here to enable SIMD and later Kokkos vectorization (efficient
 * loops). We need this to be as lightweight as possible and we have to make sure it is trivially
 * copyable.
 *
 * Another reason why this is a little complicated is that we would like to make sure
 * the derived classes can use their own member variables (like a power variable for inverse
 * distance weighting, or a limiter variable etc).
 */
class FVInterpolationMethod : public MooseObject
{
public:
  static InputParameters validParams();

  FVInterpolationMethod(const InputParameters & params);

  /**
   * Lightweight callable container used to evaluate the interpolation at a face.
   */
  struct FaceInterpolator
  {
    /// Function pointer used for the face evaluation
    using Eval = Real (*)(const FVInterpolationMethod &, const FaceInfo &, Real, Real);

    /// Pointer to the interpolation method owning the callable
    const FVInterpolationMethod * object = nullptr;

    /// Function pointer that performs the interpolation
    Eval eval = nullptr;

    /// Used for checking if the interpolator exists before we evaluate it
    bool valid() const { return object && eval; }

    /// Operator for convenient evaluation, just calls eval with forwarded arguments
    Real operator()(const FaceInfo & face, Real elem_value, Real neighbor_value) const
    {
      mooseAssert(valid(), "Attempting to call an empty interpolation handle");
      return eval(*object, face, elem_value, neighbor_value);
    }
  };

  /**
   * @return The face interpolation callable associated with this user object.
   */
  const FaceInterpolator & faceInterpolator() const { return _face_interpolator; }

protected:
  /**
   * Here is the call wrapper, this makes everything trivially copiable. This also
   * makes sure that we can use parameters that only live in the context of the
   * given interpolation method.
   */
  template <typename Derived>
  static Real callInterpolate(const FVInterpolationMethod & method,
                              const FaceInfo & face,
                              const Real elem_value,
                              const Real neighbor_value)
  {
    return static_cast<const Derived &>(method).interpolate(face, elem_value, neighbor_value);
  }

  /**
   * Utility to build an interpolation function for Derived classes.
   */
  template <typename Derived>
  FaceInterpolator buildFaceInterpolator() const
  {
    FaceInterpolator interpolator;
    interpolator.object = this;

    // We are calling the wrapper here so that the interpolation function
    // can access the member variables (limiters, manual weights etc)
    interpolator.eval = &FVInterpolationMethod::callInterpolate<Derived>;
    return interpolator;
  }

  /**
   * Save a fully constructed interpolator so kernels can evaluate it later.
   */
  void setFaceInterpolator(FaceInterpolator interpolator) { _face_interpolator = interpolator; }

private:
  FaceInterpolator _face_interpolator;
};
