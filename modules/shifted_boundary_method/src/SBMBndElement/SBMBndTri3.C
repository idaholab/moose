//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMBndTri3.h"
#include "Ball.h"
#include "LineSegment.h"

SBMBndTri3::SBMBndTri3(const Elem * elem) : SBMBndElementBase(elem)
{
  mooseAssert(elem->type() == TRI3, "Element must be of type TRI3");
}

const Point
SBMBndTri3::computeNormal() const
{
  const auto & p0 = _elem->point(0);
  const auto & p1 = _elem->point(1);
  const auto & p2 = _elem->point(2);

  const auto v1 = p1 - p0;
  const auto v2 = p2 - p0;

  auto n = v1.cross(v2);
  n /= n.norm();

  return n;
}

Ball
SBMBndTri3::computeBoundingBall() const
{
  // Compute centroid of triangle
  const auto & centroid = _elem->vertex_average();

  const auto & A = _elem->point(0);
  const auto & B = _elem->point(1);
  const auto & C = _elem->point(2);

  // Compute maximum squared distance from center to any vertex
  Real max_sq_dist = 0.0;
  for (const auto * p : {&A, &B, &C})
  {
    Real dx = p->operator()(0) - centroid(0);
    Real dy = p->operator()(1) - centroid(1);
    Real dz = p->operator()(2) - centroid(2);
    Real dist_sq = dx * dx + dy * dy + dz * dz;
    if (dist_sq > max_sq_dist)
      max_sq_dist = dist_sq;
  }

  Real radius = std::sqrt(max_sq_dist) * 1.01; // add epsilon buffer
  return Ball(centroid, radius);
}

bool
SBMBndTri3::intercepted(const LineSegment & line_segment) const
{
  const auto & a = line_segment.start();
  const auto & b = line_segment.end();

  // (a) Build basic vectors and choose a robust tolerance
  const Point dir = b - a;
  const Point v0 = _elem->point(0);
  const Point v1 = _elem->point(1);
  const Point v2 = _elem->point(2);

  const Point edge1 = v1 - v0;
  const Point edge2 = v2 - v0;
  constexpr Real eps = libMesh::TOLERANCE;

  // (b) Test for parallel or degenerate configuration
  const Point pvec = dir.cross(edge2);
  const Real det = edge1 * pvec;
  if (std::abs(det) < eps)
    return false; // ray nearly parallel to triangle

  const Real inv_det = 1.0 / det;

  // (c) Compute barycentric coordinate u and check 0 <= u <= 1
  const Point tvec = a - v0;
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
  return true;
}
