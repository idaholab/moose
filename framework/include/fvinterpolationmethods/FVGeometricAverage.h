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
#include "MooseTypes.h"

/**
 * Simple linear interpolation that uses the geometric weighting stored on FaceInfo.
 */
class FVGeometricAverage : public FVInterpolationMethod
{
public:
  static InputParameters validParams();

  FVGeometricAverage(const InputParameters & params);

  struct DeviceData
  {
  };

  static Real
  interpolate(const DeviceData &, const FaceInfo & face, Real elem_value, Real neighbor_value);

  static AdvectedSystemContribution advectedInterpolate(const DeviceData &,
                                                        const FaceInfo & face,
                                                        Real,
                                                        Real,
                                                        const VectorValue<Real> *,
                                                        const VectorValue<Real> *,
                                                        Real);

  static Real advectedInterpolateValue(const DeviceData &,
                                       const FaceInfo & face,
                                       Real elem_value,
                                       Real neighbor_value,
                                       const VectorValue<Real> *,
                                       const VectorValue<Real> *,
                                       Real);

  /**
   * Interpolate using FaceInfo's geometric weight.
   * @param face The face being interpolated.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   */
  Real interpolate(const FaceInfo & face, Real elem_value, Real neighbor_value) const;

  /**
   * Advected variant: returns matrix weights and no deferred correction.
   * @param face The face being interpolated.
   */
  AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                 Real,
                                                 Real,
                                                 const VectorValue<Real> *,
                                                 const VectorValue<Real> *,
                                                 Real) const;

  /**
   * Advected variant returning only the face value.
   * @param face The face being interpolated.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   */
  Real advectedInterpolateValue(const FaceInfo & face,
                                Real elem_value,
                                Real neighbor_value,
                                const VectorValue<Real> *,
                                const VectorValue<Real> *,
                                Real) const;
};
