//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Local includes
#include "RayTracingCommon.h"

// MOOSE includes
#include "MooseTypes.h"

// Forward declarations
namespace libMesh
{
class Elem;
}

/**
 * Struct for containing the necessary information about a cached neighbor for ray tracing
 */
struct NeighborInfo
{
  /**
   * Constructor without bounds (used for neighbors that exist only at a point)
   */
  NeighborInfo(const Elem * const elem, const std::vector<unsigned short> & sides)
    : _elem(elem),
      _sides(sides),
      _side_normals(sides.size(), RayTracingCommon::invalid_point),
      _lower_bound(-1),
      _upper_bound(-1),
      _valid(true)
  {
  }

  /**
   * Constructor with bounds (used for neighbors that span an edge)
   */
  NeighborInfo(const Elem * const elem,
               const std::vector<unsigned short> & sides,
               const Real lower_bound,
               const Real upper_bound)
    : _elem(elem),
      _sides(sides),
      _side_normals(sides.size(), RayTracingCommon::invalid_point),
      _lower_bound(lower_bound),
      _upper_bound(upper_bound),
      _valid(true)
  {
  }

  /// The element
  const Elem * const _elem;
  /// The sides on the element that the neighboring portion is contained in
  const std::vector<unsigned short> _sides;
  /// The normals of each side in _sides
  std::vector<Point> _side_normals;
  /// The lower bound (used for neighbors that span an edge)
  const Real _lower_bound;
  /// The lower bound (used for neighbors that span an edge)
  const Real _upper_bound;
  /// Whether or not this neighbor is valid (needed for neighbors that span an edge)
  bool _valid;
};
