//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#pragma once

#include "MooseTypes.h"

class FaceInfo;

/**
 * Interpolation that can provide different face values for the element and neighbor cells.
 */
class FVTwoSidedFaceInterpolation
{
public:
  struct FaceValues
  {
    Real elem;
    Real neighbor;
  };

  virtual ~FVTwoSidedFaceInterpolation() = default;

  /**
   * Interpolate the values used by the element and neighbor cells at an internal face.
   */
  virtual FaceValues
  twoSidedInterpolate(const FaceInfo & face, Real elem_value, Real neighbor_value) const = 0;
};
