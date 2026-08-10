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
   * Determine a face argument for evaluating a functor on a face. If the functor is defined on a
   * single side, that side is selected. If it is defined on both sides, the face is left unsided.
   */
  template <typename FunctorType>
  Moose::FaceArg
  functorFaceArg(const FunctorType & functor,
                 const FaceInfo & fi,
                 Moose::FV::LimiterType limiter_type = Moose::FV::LimiterType::CentralDifference,
                 bool correct_skewness = false,
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

protected:
  /**
   * Determine the single sided face argument when evaluating a functor on a face.
   * @param fi the FaceInfo for this face
   * @param limiter_type the limiter type, to be specified if more than the default average
   *        interpolation is required for the parameters of the functor
   * @param correct_skewness whether to perform skew correction at the face
   */
  Moose::FaceArg singleSidedFaceArg(
      const FaceInfo * fi,
      Moose::FV::LimiterType limiter_type = Moose::FV::LimiterType::CentralDifference,
      bool correct_skewness = false,
      const Moose::StateArg * state_limiter = nullptr) const;
};

inline Moose::FaceArg
FaceArgProducerInterface::makeCDFace(const FaceInfo & fi, const bool correct_skewness) const
{
  return makeFace(fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness);
}

inline Moose::FaceArg
FaceArgProducerInterface::singleSidedFaceArg(const FaceInfo * fi,
                                             const Moose::FV::LimiterType limiter_type,
                                             const bool correct_skewness,
                                             const Moose::StateArg * state_limiter) const
{
  mooseAssert(fi, "FaceInfo should not be null!");
  return makeFace(*fi, limiter_type, true, correct_skewness, state_limiter);
}

template <typename FunctorType>
Moose::FaceArg
FaceArgProducerInterface::functorFaceArg(const FunctorType & functor,
                                         const FaceInfo & fi,
                                         const Moose::FV::LimiterType limiter_type,
                                         const bool correct_skewness,
                                         const Moose::StateArg * state_limiter) const
{
  auto face = makeFace(fi, limiter_type, true, correct_skewness, state_limiter);
  const auto on_elem = functor.hasFaceSide(fi, true);
  const auto on_neighbor = functor.hasFaceSide(fi, false);

  if (on_elem && on_neighbor)
    face.face_side = nullptr;
  else if (on_elem)
    face.face_side = fi.elemPtr();
  else if (on_neighbor)
    face.face_side = fi.neighborPtr();
  else
    mooseError(
        "The functor '", functor.functorName(), "' is not defined on either side of the face.");

  return face;
}
