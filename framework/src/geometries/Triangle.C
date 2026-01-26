//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Triangle.h"
#include "LineSegment.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"

Triangle::Triangle(const Point & p0, const Point & p1, const Point & p2) : _p0(p0), _p1(p1), _p2(p2)
{
}

Point
Triangle::normal() const
{
  const auto v1 = _p1 - _p0;
  const auto v2 = _p2 - _p0;

  auto n = v1.cross(v2);
  n /= n.norm();

  return n;
}

bool
Triangle::intersect(const LineSegment & line_segment, Point & intersect_p) const
{
  const auto & a = line_segment.start();
  const auto & b = line_segment.end();

  // (a) Build basic vectors and choose a robust tolerance
  const Point dir = b - a;

  const Point edge1 = _p1 - _p0;
  const Point edge2 = _p2 - _p0;
  constexpr Real eps = libMesh::TOLERANCE;

  // (b) Test for parallel or degenerate configuration
  const Point pvec = dir.cross(edge2);
  const Real det = edge1 * pvec;
  if (std::abs(det) < eps)
    return false; // ray nearly parallel to triangle

  const Real inv_det = 1.0 / det;

  // (c) Compute barycentric coordinate u and check 0 <= u <= 1
  const Point tvec = a - _p0;
  const Real u = (tvec * pvec) * inv_det;
  if (!MooseUtils::absoluteFuzzyGreaterEqual(u, 0.0, eps) ||
      !MooseUtils::absoluteFuzzyLessEqual(u, 1.0, eps))
    return false;

  // (d) Compute barycentric coordinate v and check 0 <= v and u + v <= 1
  const Point qvec = tvec.cross(edge1);
  const Real v = (dir * qvec) * inv_det;
  if (!MooseUtils::absoluteFuzzyGreaterEqual(v, 0.0, eps) ||
      !MooseUtils::absoluteFuzzyLessEqual(u + v, 1.0, eps))
    return false;

  // (e) Locate intersection along line segment (0 <= t <= 1 constrains to a--b)
  const Real t = (edge2 * qvec) * inv_det;
  if (!MooseUtils::absoluteFuzzyGreaterEqual(t, 0.0, eps) ||
      !MooseUtils::absoluteFuzzyLessEqual(t, 1.0, eps))
    return false;

  // (f) Intersection lies inside both the triangle and the segment
  intersect_p = a + dir * t;
  return true;
}
