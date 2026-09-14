//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

#include "libmesh/int_range.h"
#include "libmesh/point.h"

namespace PolygonAreaTestUtils
{

/**
 * Twice the signed area of a polygon in the xy plane, which is positive when its corners are
 * ordered counter-clockwise. Computed here rather than taken from the object under test so that a
 * test checks its own input against the ordering that object requires.
 * @param polygon_points the corners of the polygon, in order around it, in any container of Points
 * @return twice the signed area
 */
template <typename T>
Real
twiceSignedArea(const T & polygon_points)
{
  Real twice_area = 0.0;
  for (const auto k : index_range(polygon_points))
  {
    const Point & current = polygon_points[k];
    const Point & next = polygon_points[(k + 1) % polygon_points.size()];
    twice_area += current(0) * next(1) - next(0) * current(1);
  }

  return twice_area;
}

}
