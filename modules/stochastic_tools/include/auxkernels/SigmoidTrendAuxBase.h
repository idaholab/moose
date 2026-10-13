//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

class KLExpansionUserObject;

/**
 * Base class for random fields sampled from a KLExpansionUserObject whose distribution parameter
 * trends with distance from a line segment, following a logistic (sigmoid) profile
 */
class SigmoidTrendAuxBase : public AuxKernel
{
public:
  static InputParameters validParams();
  SigmoidTrendAuxBase(const InputParameters & parameters);

protected:
  /// Shortest distance from p to the segment between start_point and end_point
  Real distanceToSegment(const Point & p) const;
  /// Logistic function of the distance from the segment, centered at _midpoint_of_sigmoid
  Real sigmoid(Real distance) const;
  /// Trend value at the given distance from the segment
  Real trendValue(Real distance) const;

  const KLExpansionUserObject & _kl_uo;

  /// Segment start point
  const Point _x1;
  /// Segment end point
  const Point _x2;

  /// Trend value on the segment
  const Real _scale_hi;
  /// Trend value far from the segment
  const Real _scale_lo;
  /// Distance at which the sigmoid is 1/2, its inflection point
  const Real _midpoint_of_sigmoid;
  /// Sigmoid steepness
  const Real _slope_at_midpoint;
};
