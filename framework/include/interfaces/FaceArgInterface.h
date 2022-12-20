//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const = 0;
};

/**
 * An interface for consumers of functor face arguments, e.g. the functors themselves
 */
class FaceArgConsumerInterface : public FaceArgInterface
{
public:
  /**
   * Examines the incoming face argument. If the producer did not indicate a sidedness to the face,
   * e.g. if the \p face_side member of the \p FaceArg is \p nullptr, then we may "modify" the
   * sidedness of the argument if we are only defined on one side of the face. If the producer \emph
   * has indicated a sidedness and we are not defined on that side, then we will error
   * @param face The face argument created by the producer, likely a residual object
   * @return a face with possibly changed sidedness depending on whether we aren't defined on both
   * sides of the face
   */
  Moose::FaceArg checkFace(const Moose::FaceArg & face) const;
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
                          const bool correct_skewness = false) const;

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

inline Moose::FaceArg
FaceArgProducerInterface::makeCDFace(const FaceInfo & fi, const bool correct_skewness) const
{
  return makeFace(fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness);
}
