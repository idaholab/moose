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
  /**
   * @return whether this object operates on the provided subdomain ID
   */
  virtual bool hasBlocks(SubdomainID sub_id) const = 0;

  virtual bool hasFaceSide(const FaceInfo & fi, const bool fi_elem_side) const
  {
    if (fi_elem_side)
      return hasBlocks(fi.elem().subdomain_id());
    else
      return fi.neighborPtr() && hasBlocks(fi.neighbor().subdomain_id());
  }

  /**
   * Create a functor face argument from provided component arguments
   * @param fi the face information object
   * @param limiter_type the limiter that defines how to perform interpolations to the faces
   * @param elem_is_upwind whether the face information element is the upwind element (the value of
   * this doesn't matter when the limiter type is CentralDifference)
   * @param correct_skewness whether to apply skew correction
   * @return the functor face argument
   */
  FaceArg makeFace(const FaceInfo & fi,
                   const LimiterType limiter_type,
                   const bool elem_is_upwind,
                   const bool correct_skewness = false,
                   const Elem * const face_side)
  {
    const bool defined_on_elem_side = hasFaceSide(fi, true);
    const bool defined_on_neighbor_side = hasFaceSide(fi, false);
    const Elem * const elem = defined_on_elem_side && defined_on_neighbor_side
                                  ? nullptr
                                  : (defined_on_elem_side ? &fi.elem() : fi.neighborPtr());
    if (!defined_on_elem_side && !defined_on_neighbor_side)
      mooseError("No definition on either side");

    if (face_side && elem != face_side)
      mooseError("Caller requested evaluation on a side we're not defined on");
    return {&fi, limiter_type, elem_is_upwind, correct_skewness, face_side};
  }

  /**
   * Make a functor face argument with a central differencing limiter, e.g. compose a face argument
   * that will tell functors to perform (possibly skew-corrected) linear interpolations from cell
   * center values to faces
   * @param fi the face information
   * @param correct_skewness whether to apply skew correction
   * @return a face argument for functors
   */
  inline FaceArg makeCDFace(const FaceInfo & fi,
                            const bool correct_skewness = false,
                            const Elem * const face_side = nullptr)
  {
    return makeFace(
        fi, LimiterType::CentralDifference, true, consumer, correct_skewness, face_side);
  }
};
