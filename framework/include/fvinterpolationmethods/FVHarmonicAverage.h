//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVInterpolationMethod.h"

/**
 * Harmonic-mean interpolation using the geometric weighting stored on FaceInfo.
 */
class FVHarmonicAverage : public FVInterpolationMethod
{
public:
  static InputParameters validParams();

  FVHarmonicAverage(const InputParameters & params);

  /**
   * Harmonic-mean interpolation using FaceInfo's geometric weight.
   * @param face The face being interpolated.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   */
  Real interpolate(const FaceInfo & face, Real elem_value, Real neighbor_value) const;
};
