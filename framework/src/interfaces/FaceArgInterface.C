//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FaceArgInterface.h"

Moose::FaceArg
FaceArgProducerInterface::makeFace(const FaceInfo & fi,
                                   const Moose::FV::LimiterType limiter_type,
                                   const bool elem_is_upwind,
                                   const bool correct_skewness,
                                   const Moose::StateArg * state_limiter) const
{
  const bool defined_on_elem_side = hasFaceSide(fi, true);
  const bool defined_on_neighbor_side = hasFaceSide(fi, false);
  const Elem * const elem = defined_on_elem_side && defined_on_neighbor_side
                                ? nullptr
                                : (defined_on_elem_side ? &fi.elem() : fi.neighborPtr());

  if (!defined_on_elem_side && !defined_on_neighbor_side)
    mooseError("No definition on either side");

  return {&fi, limiter_type, elem_is_upwind, correct_skewness, elem, state_limiter};
}
