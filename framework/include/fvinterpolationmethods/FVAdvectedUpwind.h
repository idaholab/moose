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

  /**
   * Return the (elem, neighbor) interpolation weights for the advected quantity.
   */
  AdvectedInterpolationResult advectedInterpolate(const FaceInfo & face,
                                                  Real elem_value,
                                                  Real neighbor_value,
                                                  const VectorValue<Real> * elem_grad,
                                                  const VectorValue<Real> * neighbor_grad,
                                                  Real mass_flux) const;
};
