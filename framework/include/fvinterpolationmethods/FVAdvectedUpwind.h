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
#include "FVInterpolationMethod.h"

/**
 * Upwind interpolation for advected quantities based on the sign of the face mass flux.
 */
class FVAdvectedUpwind : public FVInterpolationMethod, public FVAdvectedInterpolationMethod
{
public:
  static InputParameters validParams();

  FVAdvectedUpwind(const InputParameters & params);

  using FVAdvectedInterpolationMethod::advectedInterpolate;
  using FVAdvectedInterpolationMethod::advectedInterpolateValue;

  AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                 Real,
                                                 Real,
                                                 const VectorValue<Real> *,
                                                 const VectorValue<Real> *,
                                                 Real mass_flux) const override;

  Real advectedInterpolateValue(const FaceInfo & face,
                                Real elem_value,
                                Real neighbor_value,
                                const VectorValue<Real> *,
                                const VectorValue<Real> *,
                                Real mass_flux) const override;
};
