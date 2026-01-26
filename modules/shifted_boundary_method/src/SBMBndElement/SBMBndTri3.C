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

SBMBndTri3::SBMBndTri3(const Elem * elem)
  : SBMBndElementBase(elem), Triangle(elem->point(0), elem->point(1), elem->point(2))
{
  mooseAssert(elem->type() == TRI3, "Element must be of type TRI3");
}

const Point
SBMBndTri3::computeNormal() const
{
  return Triangle::normal();
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
SBMBndTri3::intersect(const LineSegment & line_segment) const
{
  Point p;
  return Triangle::intersect(line_segment, p);
}
