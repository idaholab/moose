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

// Moose includes
#include "ElementsIntersectedByPlane.h"

// libMesh includes
#include "libmesh/plane.h"
#include "libmesh/point.h"
#include "libmesh/mesh.h"
#include "libmesh/elem.h"

#include <algorithm>

namespace Moose
{

void
findElementsIntersectedByPlane(const Plane & plane,
                               const MeshBase & mesh,
                               std::vector<const Elem *> & intersected_elems)
{
  // Loop over all elements to find elements intersected by the plane
  MeshBase::const_element_iterator el = mesh.elements_begin();
  const MeshBase::const_element_iterator end_el = mesh.elements_end();
  for (; el != end_el; ++el)
  {
    const Elem * elem = *el;
    bool intersected = false;

    // Check whether the first node of this element is below or above the plane
    const Node & node0 = elem->node_ref(0);
    bool node0_above_plane = plane.above_surface(node0);

    // Loop over the rest of the nodes and check if any node is on the other side of the plane
    for (unsigned int i = 1; i < elem->n_nodes(); ++i)
    {
      const Node & node = elem->node_ref(i);

      bool node_above_plane = plane.above_surface(node);
      if (node0_above_plane != node_above_plane)
        intersected = true;
    }

    if (intersected)
      intersected_elems.push_back(elem);
  }
}

void
elementsIntersectedByPlane(const Point & p0,
                           const Point & normal,
                           const MeshBase & mesh,
                           std::vector<const Elem *> & intersected_elems)
{
  // Make sure our list is clear
  intersected_elems.clear();

  // Create plane from point and normal:
  Plane plane(p0, normal);

  // Find 'em!
  findElementsIntersectedByPlane(plane, mesh, intersected_elems);
}

void
elementsIntersectedByPlane(const Point & p0,
                           const Point & p1,
                           const Point & p2,
                           const MeshBase & mesh,
                           std::vector<const Elem *> & intersected_elems)
{
  // Make sure our list is clear
  intersected_elems.clear();

  // Create plane from three points:
  Plane plane(p0, p1, p2);

  // Find 'em!
  findElementsIntersectedByPlane(plane, mesh, intersected_elems);
}
}
