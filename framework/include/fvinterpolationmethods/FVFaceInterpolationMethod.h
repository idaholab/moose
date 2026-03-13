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
};
