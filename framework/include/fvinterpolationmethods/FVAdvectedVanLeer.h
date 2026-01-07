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
#include "Limiter.h"
#include <memory>

/**
 * TVD Van Leer interpolation for advected quantities.
 * Produces weights based on a limiter computed from cell gradients.
 */
class FVAdvectedVanLeer : public FVInterpolationMethod
{
public:
  static InputParameters validParams();

  FVAdvectedVanLeer(const InputParameters & params);

  /**
   * Return interpolation weights computed with a Van Leer limiter.
   * Uses deferred correction: matrix weights are upwind and high-order weights
   * are always provided for the correction.
   */
  AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                 Real elem_value,
                                                 Real neighbor_value,
                                                 const VectorValue<Real> * elem_grad,
                                                 const VectorValue<Real> * neighbor_grad,
                                                 Real mass_flux) const;

  /**
   * Return only the high-order face value for the advected quantity.
   */
  Real advectedInterpolateValue(const FaceInfo & face,
                                Real elem_value,
                                Real neighbor_value,
                                const VectorValue<Real> * elem_grad,
                                const VectorValue<Real> * neighbor_grad,
                                Real mass_flux) const;

private:
  const std::unique_ptr<Moose::FV::Limiter<Real>> _limiter;
};
