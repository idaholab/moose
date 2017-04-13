/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef RAYTRACING_H
#define RAYTRACING_H

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

#endif // RAYTRACING_H
