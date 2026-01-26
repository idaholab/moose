//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/point.h"

class LineSegment;

/**
 * Triangle geometry helper.
 */
class Triangle
{
public:
  Triangle() = default;
  Triangle(const libMesh::Point & p0, const libMesh::Point & p1, const libMesh::Point & p2);

  virtual ~Triangle() = default;

  /**
   * Normal vector of the triangle.
   */
  libMesh::Point normal() const;

  /**
   * Check if a line segment intersects this triangle.
   */
  bool intersect(const LineSegment & l, libMesh::Point & intersect_p) const;

private:
  libMesh::Point _p0, _p1, _p2;
};
