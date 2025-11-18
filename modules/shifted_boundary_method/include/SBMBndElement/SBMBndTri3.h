//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SBMBndElementBase.h"

/// Derived class for 3-node triangular elements (Tri3)
class SBMBndTri3 : public SBMBndElementBase
{
public:
  /// Constructor
  explicit SBMBndTri3(const Elem * elem);

  /// Check if a line segment (a-b) intersects this triangle
  bool intercepted(const Point & a, const Point & b) const override;

  /**
   * Compute the bounding ball (sphere) of the triangle
   * This function computes an approximate bounding ball of a triangle by taking the triangle's
   * centroid as the center, and setting the radius to be slightly larger than the farthest vertex
   * distance from the centroid.
   */
  std::pair<Point, Real> computeBoundingBall() const override;

protected:
  /// Compute the normal vector of the triangle
  const Point computeNormal() const override;
  Point intersectPoint(const Point & a, const Point & b, bool strict_inside) const;
};
