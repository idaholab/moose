//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolygonUtils.h"

#include "GeometryUtils.h"
#include "LineSegment.h"
#include "MooseUtils.h"

#include <cmath>

using namespace libMesh;

namespace polygon_utils
{

void
addUniquePoint(std::vector<Point> & points, const Point & point, const Real length_tol)
{
  for (const auto & existing_point : points)
    if (MooseUtils::absoluteFuzzyEqual(existing_point, point, length_tol))
      return;

  points.push_back(point);
}

bool
pointOnSegment2D(const Point & point, const Point & a, const Point & b)
{
  if (a.absolute_fuzzy_equals(b))
    return a.absolute_fuzzy_equals(point);

  return LineSegment(a, b).contains_point(point);
}

void
addSegmentIntersections2D(std::vector<Point> & points,
                          const Point & p0,
                          const Point & p1,
                          const Point & q0,
                          const Point & q1,
                          const Real length_tol)
{
  if (p0.absolute_fuzzy_equals(p1))
  {
    if (pointOnSegment2D(p0, q0, q1))
      addUniquePoint(points, p0, length_tol);

    return;
  }

  if (q0.absolute_fuzzy_equals(q1))
  {
    if (pointOnSegment2D(q0, p0, p1))
      addUniquePoint(points, q0, length_tol);

    return;
  }

  // Collinear segments meet along a whole stretch rather than at a point, so the end points of the
  // overlap are what the caller needs
  if (geom_utils::arePointsColinear(p0, p1, q0) && geom_utils::arePointsColinear(p0, p1, q1))
  {
    if (pointOnSegment2D(q0, p0, p1))
      addUniquePoint(points, q0, length_tol);
    if (pointOnSegment2D(q1, p0, p1))
      addUniquePoint(points, q1, length_tol);
    if (pointOnSegment2D(p0, q0, q1))
      addUniquePoint(points, p0, length_tol);
    if (pointOnSegment2D(p1, q0, q1))
      addUniquePoint(points, p1, length_tol);

    return;
  }

  Point intersection = p0;

  // LineSegment::intersect() works with the infinite lines, so the result still has to be checked
  // against both segments
  if (LineSegment(p0, p1).intersect(LineSegment(q0, q1), intersection) &&
      pointOnSegment2D(intersection, p0, p1) && pointOnSegment2D(intersection, q0, q1))
    addUniquePoint(points, intersection, length_tol);
}

bool
pointInPolygon2D(const Point & point, const std::vector<Point> & polygon)
{
  if (polygon.size() < 3)
    return false;

  bool inside = false;

  for (const auto i : index_range(polygon))
  {
    const auto j = (i + polygon.size() - 1) % polygon.size();
    const Point & pi = polygon[i];
    const Point & pj = polygon[j];

    if (pointOnSegment2D(point, pj, pi))
      return true;

    // Count the crossings of a ray cast from the point along -x: an odd count means inside
    if ((pi(1) > point(1)) != (pj(1) > point(1)) &&
        point(0) < (pj(0) - pi(0)) * (point(1) - pi(1)) / (pj(1) - pi(1)) + pi(0))
      inside = !inside;
  }

  return inside;
}

std::size_t
fanApexOffset(const std::vector<Point> & points, const Real area_tol)
{
  const auto n_points = points.size();
  const auto n_degenerate_triangles = [&](const std::size_t apex)
  {
    unsigned int n_degenerate = 0;

    for (const auto i : make_range(std::size_t(1), n_points - 1))
      if (std::abs(geom_utils::signedArea2D(points[apex],
                                            points[(apex + i) % n_points],
                                            points[(apex + i + 1) % n_points])) <= area_tol)
        ++n_degenerate;

    return n_degenerate;
  };

  for (const auto apex : index_range(points))
    if (n_degenerate_triangles(apex) == 0)
      return apex;

  return 0;
}

} // end of namespace polygon_utils
