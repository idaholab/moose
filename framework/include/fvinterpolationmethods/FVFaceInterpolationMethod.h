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
 * Interface for interpolation methods that produce a scalar face value from adjacent cell values.
 */
class FVFaceInterpolationMethod
{
public:
  /**
   * Face interpolation operation for this method.
   */
  virtual Real interpolate(const FaceInfo & face, Real elem_value, Real neighbor_value) const = 0;

  /**
   * Convenience overload that evaluates a scalar Moose functor at the adjacent cell centers and
   * then applies this interpolation method.
   * This requires a two-sided internal face.
   */
  Real interpolate(const Moose::FunctorBase<Real> & functor,
                   const FaceInfo & face,
                   const Moose::StateArg & state) const
  {
    mooseAssert(face.neighborPtr(),
                "Face interpolation with a Moose functor requires a two-sided internal face.");

    const Real elem_value = functor(Moose::ElemArg{face.elemPtr(), false}, state);
    const Real neighbor_value = functor(Moose::ElemArg{face.neighborPtr(), false}, state);
    return interpolate(face, elem_value, neighbor_value);
  }
};
