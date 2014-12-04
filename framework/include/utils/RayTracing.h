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

#include "Moose.h"
#include "LineSegment.h"

// libMesh includes
#include "libmesh/point.h"

// forward declares
class LineSegment;
class MooseMesh;

namespace libMesh {
  class Point;
  class Plane;
  class MeshBase;
  class Elem;
}

namespace Moose
{
/**
 * Find all of the elements intersected by a line.
 * The line is given as the beginning and ending points
 * @param p0 The beginning of the line
 * @param p1 The end of the line
 * @poaram intersected_elems The elements intersected by the line.  Will be empty if there are no intersections.
 */
void elementsIntersectedByLine(const Point & p0, const Point & p1, const MeshBase & mesh, std::vector<Elem *> & intersected_elems);
}


#endif // RAYTRACING_H
