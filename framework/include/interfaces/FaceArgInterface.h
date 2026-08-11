//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "FaceInfo.h"
#include "MooseFunctorArguments.h"
#include "libmesh/elem.h"

/**
 * A base class interface for both producers and consumers of functor face arguments, e.g. residual
 * objects/postprocessors and functors respectively
 */
class FaceArgInterface
{
public:
  virtual ~FaceArgInterface() = default;
  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const = 0;
};

/**
 * An interface for producers of functor face arguments, e.g. objects such as residual objects and
 * postprocessors
 */
class FaceArgProducerInterface : public FaceArgInterface
{
public:
  /**
   * Create a functor face argument from provided component arguments
   * @param fi the face information object
   * @param limiter_type the limiter that defines how to perform interpolations to the faces
   * @param elem_is_upwind whether the face information element is the upwind element (the value
   * of this doesn't matter when the limiter type is CentralDifference)
   * @param correct_skewness whether to apply skew correction
   * @return the functor face argument
   */
  Moose::FaceArg makeFace(const FaceInfo & fi,
                          const Moose::FV::LimiterType limiter_type,
                          const bool elem_is_upwind,
                          const bool correct_skewness = false,
                          const Moose::StateArg * state_limiter = nullptr) const;

  /**
   * Create a face argument according to where the supplied functor is defined.
   *
   * Unlike the overload above, which selects the face side according to the side reported by the
   * face-producing object, this overload determines the face side from the supplied functor. This
   * allows the resulting FaceArg to evaluate a functor that is defined only on the opposite side of
   * the face from the producer.
   *
   * If the functor is defined on both sides, the face argument is left unsided. If it is defined
   * on only one side, that side is selected. An error is produced if the functor is defined on
   * neither side.
   *
   * @param functor the functor whose sidedness is queried
   * @param fi the face information object
   * @param limiter_type the limiter that defines how to perform interpolations to the face
   * @param elem_is_upwind whether the face information element is the upwind element
   * @param correct_skewness whether to apply skew correction
   * @param state_limiter optional state used by the limiter
   * @return the functor face argument
   */
  template <typename FunctorType>
  Moose::FaceArg makeFace(const FunctorType & functor,
                          const FaceInfo & fi,
                          const Moose::FV::LimiterType limiter_type,
                          const bool elem_is_upwind,
                          const bool correct_skewness = false,
                          const Moose::StateArg * state_limiter = nullptr) const;

  /**
   * Make a functor face argument with a central differencing limiter, e.g. compose a face
   * argument that will tell functors to perform (possibly skew-corrected) linear interpolations
   * from cell center values to faces
   * @param fi the face information
   * @param correct_skewness whether to apply skew correction
   * @return a face argument for functors
   */
  Moose::FaceArg makeCDFace(const FaceInfo & fi, const bool correct_skewness = false) const;
};

template <typename FunctorType>
inline Moose::FaceArg
FaceArgProducerInterface::makeFace(const FunctorType & functor,
                                   const FaceInfo & fi,
                                   const Moose::FV::LimiterType limiter_type,
                                   const bool elem_is_upwind,
                                   const bool correct_skewness,
                                   const Moose::StateArg * state_limiter) const
{
  const bool defined_on_elem_side = functor.hasFaceSide(fi, true);
  const bool defined_on_neighbor_side = functor.hasFaceSide(fi, false);

  if (!defined_on_elem_side && !defined_on_neighbor_side)
    mooseError(
        "The functor '", functor.functorName(), "' is not defined on either side of the face.");

  const Elem * const face_side = defined_on_elem_side && defined_on_neighbor_side
                                     ? nullptr
                                     : (defined_on_elem_side ? fi.elemPtr() : fi.neighborPtr());

  return {&fi, limiter_type, elem_is_upwind, correct_skewness, face_side, state_limiter};
}

inline Moose::FaceArg
FaceArgProducerInterface::makeCDFace(const FaceInfo & fi, const bool correct_skewness) const
{
  return makeFace(fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness);
}
