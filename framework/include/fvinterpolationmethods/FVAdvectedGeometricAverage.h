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
 * Geometric averaging for advected quantities using FaceInfo::gC weighting.
 */
class FVAdvectedGeometricAverage : public FVInterpolationMethod
{
public:
  static InputParameters validParams();

  FVAdvectedGeometricAverage(const InputParameters & params);

  /**
   * Return (elem, neighbor) interpolation weights using the geometric factor.
   */
  AdvectedInterpolationResult advectedInterpolate(const FaceInfo & face,
                                                  Real elem_value,
                                                  Real neighbor_value,
                                                  const VectorValue<Real> * elem_grad,
                                                  const VectorValue<Real> * neighbor_grad,
                                                  Real mass_flux) const;
};
