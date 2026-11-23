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

} // end of namespace polygon_utils
