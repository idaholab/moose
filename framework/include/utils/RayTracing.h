//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"

// Forward declarations
class LineSegment;
class MooseMesh;

namespace libMesh
{
class Elem;
class MeshBase;
class Plane;
class Point;
class PointLocatorBase;
}

namespace Moose
{
/**
 * Find all of the elements intersected by a line.
 * The line is given as the beginning and ending points
 * @param p0 The beginning of the line
 * @param p1 The end of the line
 * @param intersected_elems The elements intersected by the line.  Will be empty if there are no
 * intersections.
 * @param segments The line segments across each element
 */
void elementsIntersectedByLine(const Point & p0,
                               const Point & p1,
                               const MeshBase & mesh,
                               const PointLocatorBase & point_locator,
                               std::vector<Elem *> & intersected_elems,
                               std::vector<LineSegment> & segments);
}
