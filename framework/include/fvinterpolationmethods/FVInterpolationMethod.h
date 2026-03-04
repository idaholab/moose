//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseObject.h"
#include "FaceInfo.h"
#include "GradientLimiterType.h"
#include "MooseError.h"

#include <utility>

/**
 * Base class for face interpolation functions used by linear FV objects.
 */
class FVInterpolationMethod : public MooseObject
{
public:
  static InputParameters validParams();

  FVInterpolationMethod(const InputParameters & params);

  /// Matrix/RHS contribution for an advected face interpolation.
  struct AdvectedSystemContribution
  {
    std::pair<Real, Real> weights_matrix;
    Real rhs_face_value = 0.0;
  };

  /**
   * Face interpolation operation for this method.
   */
  virtual Real interpolate(const FaceInfo & face, Real elem_value, Real neighbor_value) const;

  /**
   * Whether this method supports face interpolation.
   */
  virtual bool supportsFaceInterpolation() const { return false; }

  /**
   * Advected interpolation operation for this method.
   */
  virtual AdvectedSystemContribution advectedInterpolate(const FaceInfo & face,
                                                         Real elem_value,
                                                         Real neighbor_value,
                                                         const VectorValue<Real> * elem_grad,
                                                         const VectorValue<Real> * neighbor_grad,
                                                         Real mass_flux) const;

  /**
   * Whether this method supports advected interpolation.
   */
  virtual bool supportsAdvectedInterpolation() const { return false; }

  /**
   * Advected interpolation operation that returns only the face value.
   */
  virtual Real advectedInterpolateValue(const FaceInfo & face,
                                        Real elem_value,
                                        Real neighbor_value,
                                        const VectorValue<Real> * elem_grad,
                                        const VectorValue<Real> * neighbor_grad,
                                        Real mass_flux) const;

  /**
   * Whether advected interpolation requires adjacent-cell gradients.
   */
  virtual bool advectedInterpolationNeedsGradients() const { return false; }

  /**
   * Limiter used by interpolations that require limited gradients.
   */
  virtual Moose::FV::GradientLimiterType gradientLimiter() const
  {
    return Moose::FV::GradientLimiterType::None;
  }
};
