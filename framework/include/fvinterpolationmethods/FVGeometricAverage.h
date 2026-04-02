//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVAdvectedInterpolationMethod.h"
#include "FVFaceInterpolationMethod.h"
#include "FVInterpolationMethod.h"
#include "MooseTypes.h"

/**
 * Simple linear interpolation that uses the geometric weighting stored on FaceInfo.
 */
class FVGeometricAverage : public FVInterpolationMethod,
                           public FVFaceInterpolationMethod,
                           public FVAdvectedInterpolationMethod
{
public:
  static InputParameters validParams();

  FVGeometricAverage(const InputParameters & params);

  using FVAdvectedInterpolationMethod::advectedInterpolate;
  using FVAdvectedInterpolationMethod::advectedInterpolateValue;
  using FVFaceInterpolationMethod::interpolate;

  Real interpolate(const FaceInfo & face, Real elem_value, Real neighbor_value) const override;

  AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                 Real,
                                                 Real,
                                                 const VectorValue<Real> *,
                                                 const VectorValue<Real> *,
                                                 Real) const override;

  Real advectedInterpolateValue(const FaceInfo & face,
                                Real elem_value,
                                Real neighbor_value,
                                const VectorValue<Real> *,
                                const VectorValue<Real> *,
                                Real) const override;
};
