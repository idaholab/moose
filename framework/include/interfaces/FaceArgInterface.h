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

class FaceArgInterface
{
public:
  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const = 0;
};

class FaceArgConsumerInterface : public FaceArgInterface
{
public:
  bool hasDiscontinuity(const FaceInfo &) const { return false; }

  Moose::FaceArg checkFace(const Moose::FaceArg & face) const
  {
    const Elem * const elem = face.face_side;
    const FaceInfo * const fi = face.fi;
    mooseAssert(fi, "face info should be non-null");
    auto ret_face = face;
    bool check_elem_def = false;
    bool check_neighbor_def = false;
    if (!elem)
    {
      if (!hasFaceSide(*fi, true))
      {
        ret_face.face_side = fi->neighborPtr();
        check_neighbor_def = true;
      }
      else if (!hasFaceSide(*fi, false))
      {
        ret_face.face_side = &fi->elem();
        check_elem_def = true;
      }
    }
    else if (elem == &fi->elem())
      check_elem_def = true;
    else
    {
      mooseAssert(elem == fi->neighborPtr(), "This has to match something");
      check_neighbor_def = true;
    }

    if (check_elem_def && !hasFaceSide(*fi, true))
      mooseError("Functor argument consumer is not defined on the element side of the face "
                 "information, but a producer has requested evaluation there");
    if (check_neighbor_def && !hasFaceSide(*fi, false))
      mooseError("Functor argument consumer is not defined on the neighbor side of the face "
                 "information, but a producer has requested evaluation there");

    return ret_face;
  }
};

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
                          const bool correct_skewness = false) const
  {
    const bool defined_on_elem_side = hasFaceSide(fi, true);
    const bool defined_on_neighbor_side = hasFaceSide(fi, false);
    const Elem * const elem = defined_on_elem_side && defined_on_neighbor_side
                                  ? nullptr
                                  : (defined_on_elem_side ? &fi.elem() : fi.neighborPtr());

    if (!defined_on_elem_side && !defined_on_neighbor_side)
      mooseError("No definition on either side");

    return {&fi, limiter_type, elem_is_upwind, correct_skewness, elem};
  }

  /**
   * Make a functor face argument with a central differencing limiter, e.g. compose a face
   * argument that will tell functors to perform (possibly skew-corrected) linear interpolations
   * from cell center values to faces
   * @param fi the face information
   * @param correct_skewness whether to apply skew correction
   * @return a face argument for functors
   */
  Moose::FaceArg makeCDFace(const FaceInfo & fi, const bool correct_skewness = false) const
  {
    return makeFace(fi, Moose::FV::LimiterType::CentralDifference, true, correct_skewness);
  }
};
