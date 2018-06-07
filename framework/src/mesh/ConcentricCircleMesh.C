//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConcentricCircleMesh.h"

#include "libmesh/mesh_generation.h"
#include "libmesh/string_to_enum.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary_base.h"
#include "libmesh/unstructured_mesh.h"

// C++ includes
#include <cmath> // provides round, not std::round (see http://www.cplusplus.com/reference/cmath/round/)

registerMooseObject("MooseApp", ConcentricCircleMesh);

template <>
InputParameters
validParams<ConcentricCircleMesh>()
{
  InputParameters params = validParams<MooseMesh>();

  params.addRequiredParam<unsigned int>("num_sectors",
                                        "Number of azimuthal sectors in each quadrant");
  params.addRangeCheckedParam<unsigned int>("extra_radial_intervals",
                                            1,
                                            "extra_radial_intervals>=1",
                                            "The number of 'outer' intervals for the circle");

  return params;
}

ConcentricCircleMesh::ConcentricCircleMesh(const InputParameters & parameters)
  : MooseMesh(parameters),
    _num_sectors(getParam<unsigned int>("num_sectors")),
    _extra_radial_intervals(getParam<unsigned int>("extra_radial_intervals"))
{
}

std::unique_ptr<MooseMesh>
ConcentricCircleMesh::safeClone() const
{
  return libmesh_make_unique<ConcentricCircleMesh>(*this);
}

void
ConcentricCircleMesh::buildMesh()
{
  // Get the actual libMesh mesh
  MeshBase & mesh = getMesh();

  // Add the points
  Node * node0 = mesh.add_point(Point(0, 0, 0), /*id=*/0);
  Node * node1 = mesh.add_point(Point(1, 0, 0), /*id=*/1);
  Node * node2 = mesh.add_point(Point(1, 1, 0), /*id=*/2);
  Node * node3 = mesh.add_point(Point(0, 1, 0), /*id=*/3);

  // Create the element
  Elem * elem = new Quad4;
  elem = mesh.add_elem(elem);

  // Set the nodes on the element
  elem->set_node(0) = node0;
  elem->set_node(1) = node1;
  elem->set_node(2) = node2;
  elem->set_node(3) = node3;

  mesh.prepare_for_use(/*skip_renumbering=*/true);
}
