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

#ifndef ELEMENTSINTERSECTEDBYPLANE_H
#define ELEMENTSINTERSECTEDBYPLANE_H

#include "Moose.h"

// libMesh includes
#include "libmesh/point.h"

// forward declares
class MooseMesh;

namespace libMesh
{
class Point;
class Plane;
class MeshBase;
class Elem;
}

namespace Moose
{
/**
 * Find all of the elements intersected by a plane.
 * The plane is given as a point and a normal vector.
 * @param p0 Point in plane.
 * @param normal Normal vector to plane.
 * @param intersected_elems The elements intersected by the plane.  Will be empty if there are no
 * intersections.
 */
void elementsIntersectedByPlane(const Point & p0,
                                const Point & normal,
                                const MeshBase & mesh,
                                std::vector<const Elem *> & intersected_elems);

/**
 * Find all of the elements intersected by a plane.
 * The plane is given as three points in the plane.
 * @param p0 Point in plane.
 * @param p1 Point in plane.
 * @param p2 Point in plane.
 * @param intersected_elems The elements intersected by the plane.  Will be empty if there are no
 * intersections.
 */
void elementsIntersectedByPlane(const Point & p0,
                                const Point & p1,
                                const Point & p2,
                                const MeshBase & mesh,
                                std::vector<const Elem *> & intersected_elems);
}

#endif // ELEMENTSINTERSECTEDBYPLANE_H
