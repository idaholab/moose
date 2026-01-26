//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMBndEdge2.h"
#include "LineSegment.h"
#include "Ball.h"

SBMBndEdge2::SBMBndEdge2(const Elem * elem) : SBMBndElementBase(elem)
{
  mooseAssert(elem->type() == EDGE2, "Element must be of type EDGE2");
  mooseAssert(MooseUtils::absoluteFuzzyEqual(elem->point(0)(2), elem->point(1)(2)),
              "Currently SBMBndEdge2 must be parallel to the x-y plane, i.e., z-coordinates of "
              "points must be equal");
}

bool
SBMBndEdge2::intercepted(const LineSegment & line_segment) const
{
  const auto & p1 = _elem->point(0);
  const auto & p2 = _elem->point(1);

  LineSegment edge_seg(p1, p2);

  Point ip;
  if (!edge_seg.intersect(line_segment, ip))
    return false;

  auto orientation = [](const Point & p, const Point & q, const Point & r)
  { return (q(1) - p(1)) * (r(0) - q(0)) - (q(0) - p(0)) * (r(1) - q(1)); };

  const bool collinear =
      MooseUtils::absoluteFuzzyEqual(orientation(p1, p2, line_segment.start()), 0) &&
      MooseUtils::absoluteFuzzyEqual(orientation(p1, p2, line_segment.end()), 0);

  if (!collinear)
    return true; // Non-collinear case: the result from edge_seg.intersect(line_segment, ip)
                 // is reliable and sufficient to confirm an intersection.

  const bool overlap = line_segment.contains_point(p1) || line_segment.contains_point(p2) ||
                       edge_seg.contains_point(line_segment.start()) ||
                       edge_seg.contains_point(line_segment.end());
  return overlap;
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

Ball
SBMBndEdge2::computeBoundingBall() const
{
  const auto & p1 = _elem->point(0);
  const auto & p2 = _elem->point(1);
  const auto center = _elem->vertex_average();
  const auto radius = 0.5 * (p2 - p1).norm();
  return Ball(center, radius);
}
