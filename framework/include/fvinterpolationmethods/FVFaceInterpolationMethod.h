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
#include "MooseFunctor.h"

/**
 * Abstract base class for interpolation methods that produce a scalar face
 * value from adjacent cell values.
 */
class FVFaceInterpolationMethod
{
public:
  /**
   * Face interpolation operation for this method.
   * @param face The face to interpolate at.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   */
  virtual Real interpolate(const FaceInfo & face, Real elem_value, Real neighbor_value) const = 0;

  /**
   * Convenience overload that evaluates a scalar Moose functor at the adjacent cell centers and
   * then applies this interpolation method.
   * This requires a two-sided internal face.
   * @param functor The function which will be interpolated onto the face.
   * @param face The face which will be use for interpolation.
   * @param state The state argument for which we are performing the interpolation.
   */
  Real interpolate(const Moose::FunctorBase<Real> & functor,
                   const FaceInfo & face,
                   const Moose::StateArg & state) const;
};
