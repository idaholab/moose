//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMBndEdge2.h"
#include "libmesh/utility.h" // for orientation and intersection utilities

SBMBndEdge2::SBMBndEdge2(const Elem * elem) : SBMBndElementBase(elem)
{
  mooseAssert(elem->type() == EDGE2, "Element must be of type EDGE2");
  mooseAssert(MooseUtils::absoluteFuzzyEqual(elem->point(0)(2), elem->point(1)(2)),
              "Currently SBMBndEdge2 must be parallel to the x-y plane, i.e., z-coordinates of "
              "points must be equal");
}

bool
SBMBndEdge2::intercepted(const Point & a, const Point & b) const
{
  const auto & p1 = _elem->point(0);
  const auto & p2 = _elem->point(1);

  // Check if line segments (a-b) and (p1-p2) intersect
  // This is a 2D geometric segment-segment intersection
  auto orientation = [](const Point & p, const Point & q, const Point & r)
  { return (q(1) - p(1)) * (r(0) - q(0)) - (q(0) - p(0)) * (r(1) - q(1)); };

  auto onSegment = [](const Point & p, const Point & q, const Point & r)
  {
    return std::min(p(0), r(0)) <= q(0) && q(0) <= std::max(p(0), r(0)) &&
           std::min(p(1), r(1)) <= q(1) && q(1) <= std::max(p(1), r(1));
  };

  Real o1 = orientation(a, b, p1);
  Real o2 = orientation(a, b, p2);
  Real o3 = orientation(p1, p2, a);
  Real o4 = orientation(p1, p2, b);

  // General case
  if ((o1 * o2 < 0) && (o3 * o4 < 0))
    return true;

  // Special Cases
  if (MooseUtils::absoluteFuzzyEqual(o1, 0) && onSegment(a, p1, b))
    return true;
  if (MooseUtils::absoluteFuzzyEqual(o2, 0) && onSegment(a, p2, b))
    return true;
  if (MooseUtils::absoluteFuzzyEqual(o3, 0) && onSegment(p1, a, p2))
    return true;
  if (MooseUtils::absoluteFuzzyEqual(o4, 0) && onSegment(p1, b, p2))
    return true;

  return false;
}

const Point
SBMBndEdge2::computeNormal() const
{
  const auto & p0 = _elem->point(0);
  const auto & p1 = _elem->point(1);

  const auto tangent = p1 - p0;

  // Rotate 90 degrees counter-clockwise (2D)
  Point n(-tangent(1), tangent(0), 0.0);
  n /= n.norm();

  return n;
}

std::pair<Point, Real>
SBMBndEdge2::computeBoundingBall() const
{
  const auto & p1 = _elem->point(0);
  const auto & p2 = _elem->point(1);
  const auto center = _elem->vertex_average();
  const auto radius = 0.5 * (p2 - p1).norm();
  return {center, radius};
}
