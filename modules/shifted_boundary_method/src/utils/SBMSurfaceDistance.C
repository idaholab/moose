//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SBMSurfaceDistance.h"
#include "SurfaceElement.h"
#include "MooseError.h"

#include "libmesh/elem.h"
#include "libmesh/string_to_enum.h"

#include <algorithm>
#include <limits>

namespace SBMUtils
{
Point
distanceFrom(const SurfaceElement & surface_elem, const Point & pt)
{
  const Elem & element = surface_elem.elem();
  const Point & normal = surface_elem.normal();

  // Precondition: this routine only handles EDGE2 and NODEELEM element sides.
  // libMesh standard elements have uniform side types, so probing side(0) is
  // sufficient. Validated once here as a debug assert; callers build these from
  // supported surface meshes.
  mooseAssert(
      [&]()
      {
        if (element.n_sides() == 0)
          return true;
        const auto t = element.build_side_ptr(0)->type();
        return t == EDGE2 || t == NODEELEM;
      }(),
      "SBMUtils::distanceFrom only handles EDGE2 and NODEELEM element sides.");

  // (a) Project pt onto the normal direction
  const auto vec_to_first = element.point(0) - pt;
  const auto scale = vec_to_first * normal;
  const auto projection = normal * scale;

  // Check if projection point lands inside the geometry
  if (element.contains_point(pt + projection))
    return projection;

  // (b) Point to closest edge or node
  Real min_dist = std::numeric_limits<Real>::max();
  Point closest_vec;

  const unsigned int n_edges = element.n_sides();
  for (unsigned int j = 0; j < n_edges; ++j)
  {
    std::unique_ptr<const Elem> curr_edge = element.build_side_ptr(j);

    switch (curr_edge->type())
    {
      case EDGE2:
      {
        const Point & p1 = *curr_edge->node_ptr(0);
        const Point & p2 = *curr_edge->node_ptr(1);

        const Point edge = p2 - p1;
        Real t = ((pt - p1) * edge) / (edge * edge);
        t = std::clamp(t, 0.0, 1.0);
        const Point proj = p1 + t * edge;
        const Real dist = (pt - proj).norm();

        if (dist < min_dist)
        {
          min_dist = dist;
          closest_vec = proj - pt;
        }
        break;
      }

      case NODEELEM:
      {
        const Point & p = *curr_edge->node_ptr(0);
        const Real dist = (pt - p).norm();
        if (dist < min_dist)
        {
          min_dist = dist;
          closest_vec = p - pt;
        }
        break;
      }

      default:
        mooseAssert(false, "unreachable: side type validated by the precondition above");
    }
  }

  return closest_vec;
}
}
