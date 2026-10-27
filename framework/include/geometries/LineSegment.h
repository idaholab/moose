//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Ball.h"
#include "DataIO.h"
#include "GeometryBase.h"

#include "json.h"
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
class LineSegment : public GeometryBase
{
public:
  LineSegment() = default;
  LineSegment(const Point & p0, const Point & p1);

  ~LineSegment() override = default;

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

  /**
   * Check if a line segment intersects a plane, and if so, return the intersection point.
   */
  bool intersect(const libMesh::Plane & pl, Point & intersect_p) const;

  /**
   * Check if a line segment intersects another line segment, and if so, return the intersection
   * point.
   */
  bool intersect(const LineSegment & l1, Point & intersect_p) const;

  /**
   * Check if a line segment intersects another line segment, without returning the intersection
   * point.
   */
  bool intersect(const LineSegment & line_segment) const override;

  /**
   * Compute a bounding ball for this line segment.
   *
   * The ball is defined by the midpoint of the segment and a radius equal to half the segment
   * length.
   */
  Ball computeBoundingBall() const override;

  /**
   * Beginning of the line segment.
   */
  const Point & start() const { return _p0; }

  /**
   * Ending of the line segment.
   */
  const Point & end() const { return _p1; }

  /**
   * Sets the beginning of the line segment.
   */
  void setStart(const Point & p0) { _p0 = p0; }

  /**
   * Sets the end of the line segment.
   */
  void setEnd(const Point & p1) { _p1 = p1; }

  /**
   * Sets the points on the line segment.
   * @param p0 The start point of the line segment
   * @param p1 The end point of the line segment
   */
  void set(const Point & p0, const Point & p1);

  /**
   * Length of segment
   */
  Real length() const { return (_p0 - _p1).norm(); }

  /**
   * normal vector of the line segment
   */
  Point normal() const;

private:
  bool closest_point(const Point & p, bool clamp_to_segment, Point & closest_p) const;

  Point _p0, _p1;
};

void dataStore(std::ostream & stream, LineSegment & l, void * context);
void dataLoad(std::istream & stream, LineSegment & l, void * context);

void to_json(nlohmann::json & json, const LineSegment & l);
