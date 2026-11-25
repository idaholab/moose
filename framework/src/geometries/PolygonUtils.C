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
#include "predicates.h"

#include <algorithm>
#include <cmath>

using namespace libMesh;

namespace polygon_utils
{

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

    if (geom_utils::pointOnSegment2D(point, pj, pi))
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

void
sortPointsCounterClockwise(std::vector<Point> & points, const Point & center)
{
  Moose::initPredicates();

  // Which side of the horizontal line through the center a point lies on. Points on the line count
  // as upper when they are to the right of the center, which puts the start of the sweep at the
  // positive x axis and keeps the two halves disjoint.
  const auto isUpperHalf = [&center](const Point & p)
  { return p(1) > center(1) || (p(1) == center(1) && p(0) >= center(0)); };

  std::sort(points.begin(),
            points.end(),
            [&center, &isUpperHalf](const Point & a, const Point & b)
            {
              const bool a_upper = isUpperHalf(a);

              if (a_upper != isUpperHalf(b))
                return a_upper;

              // Within one half plane the sweep spans less than pi, so b is further along it
              // exactly when it lies clockwise of a
              const double center_xy[2] = {center(0), center(1)};
              const double a_xy[2] = {a(0), a(1)};
              const double b_xy[2] = {b(0), b(1)};
              const auto orientation = moose_orient2d(center_xy, a_xy, b_xy);

              if (orientation != 0.0)
                return orientation > 0.0;

              // Collinear with the center, so order by position to keep this a strict weak
              // ordering. Two points at the same position compare equal, as they must.
              return std::make_pair(a(0), a(1)) < std::make_pair(b(0), b(1));
            });
}

} // end of namespace polygon_utils
