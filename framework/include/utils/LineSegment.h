//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Moose.h" // using namespace libMesh

#include "libmesh/point.h"

// forward declarations
namespace libMesh
{
class Plane;
}

/**
 * The LineSegment class is used by the LineMaterialSamplerBase class
 * and for some ray tracing stuff.
 */
class LineSegment
{
public:
  LineSegment(const Point & p0, const Point & p1);

  virtual ~LineSegment() = default;

  /**
   * Returns the closest point on the LineSegment
   * to the passed in point.  Note that the closest point may be
   * one of the ends of the LineSegment.
   */
  Point closest_point(const Point & p) const;

  /**
   * Finds the closest point on the Line determined by the
   * Line Segments.  Returns a boolean indicating whether
   * that normal point is within the LineSegment or not
   */
  bool closest_normal_point(const Point & p, Point & closest_p) const;

  /**
   * Determines whether a point is in a line segment or not
   */
  bool contains_point(const Point & p) const;

  bool intersect(const Plane & pl, Point & intersect_p) const;

  bool intersect(const LineSegment & l1, Point & intersect_p) const;

  /**
   * Beginning of the line segment.
   */
  const Point & start() const { return _p0; }

  /**
   * Ending of the line segment.
   */
  const Point & end() const { return _p1; }

  /**
   * Length of segment
   */
  Real length() const { return (_p0 - _p1).norm(); }

private:
  bool closest_point(const Point & p, bool clamp_to_segment, Point & closest_p) const;

  Point _p0, _p1;
};
