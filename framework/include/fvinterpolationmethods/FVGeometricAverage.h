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
 * Simple linear interpolation that uses the geometric weighting stored on FaceInfo.
 */
class FVGeometricAverage : public FVInterpolationMethod
{
public:
  static InputParameters validParams();

  FVGeometricAverage(const InputParameters & params);

  Real interpolate(const FaceInfo & face, Real elem_value, Real neighbor_value) const;
};
