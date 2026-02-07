//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

class LineSegment;
class Ball;

/**
 * Base class for geometry primitives.
 */
class GeometryBase
{
public:
  virtual ~GeometryBase() = default;

  /**
   * Check if a line segment intersects this geometry.
   */
  virtual bool intersect(const LineSegment & line_segment) const = 0;

  /**
   * Compute a bounding ball for this geometry.
   * Sphere in 3D, circle in 2D. The ball should fully contain the geometry.
   */
  virtual Ball computeBoundingBall() const = 0;
};
