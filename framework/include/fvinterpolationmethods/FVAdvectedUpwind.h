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
 * Upwind interpolation for advected quantities based on the sign of the face mass flux.
 */
class FVAdvectedUpwind : public FVInterpolationMethod
{
public:
  static InputParameters validParams();

  FVAdvectedUpwind(const InputParameters & params);

  struct DeviceData
  {
  };

  static AdvectedSystemContribution advectedInterpolate(const DeviceData &,
                                                        const FaceInfo & face,
                                                        Real,
                                                        Real,
                                                        const VectorValue<Real> *,
                                                        const VectorValue<Real> *,
                                                        Real mass_flux);

  static Real advectedInterpolateValue(const DeviceData &,
                                       const FaceInfo & face,
                                       Real elem_value,
                                       Real neighbor_value,
                                       const VectorValue<Real> *,
                                       const VectorValue<Real> *,
                                       Real mass_flux);

  /**
   * Return the (elem, neighbor) interpolation weights for the advected quantity.
   * @param face The face being interpolated.
   * @param mass_flux Face mass flux for determining upwind direction.
   */
  AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                 Real,
                                                 Real,
                                                 const VectorValue<Real> *,
                                                 const VectorValue<Real> *,
                                                 Real mass_flux) const;

  /**
   * Return only the upwind face value for the advected quantity.
   * @param face The face being interpolated.
   * @param elem_value Element-side scalar value.
   * @param neighbor_value Neighbor-side scalar value.
   * @param mass_flux Face mass flux for determining upwind direction.
   */
  Real advectedInterpolateValue(const FaceInfo & face,
                                Real elem_value,
                                Real neighbor_value,
                                const VectorValue<Real> *,
                                const VectorValue<Real> *,
                                Real mass_flux) const;
};
