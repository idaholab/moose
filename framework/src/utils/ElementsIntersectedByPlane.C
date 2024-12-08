//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Moose includes
#include "ElementsIntersectedByPlane.h"

#include "libmesh/plane.h"
#include "libmesh/point.h"
#include "libmesh/mesh.h"
#include "libmesh/elem.h"

#include <algorithm>

using namespace libMesh;

namespace Moose
{

void
findElementsIntersectedByPlane(const libMesh::Plane & plane,
                               const MeshBase & mesh,
                               std::vector<const Elem *> & intersected_elems)
{
  // Loop over all elements to find elements intersected by the plane
  for (const auto & elem : mesh.element_ptr_range())
  {
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
  libMesh::Plane plane(p0, normal);

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
  libMesh::Plane plane(p0, p1, p2);

  // Find 'em!
  findElementsIntersectedByPlane(plane, mesh, intersected_elems);
}
}
