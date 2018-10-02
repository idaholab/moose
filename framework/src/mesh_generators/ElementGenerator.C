//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementGenerator.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/face_quad4.h"

registerMooseObject("MooseApp", ElementGenerator);

template <>
InputParameters
validParams<ElementGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();

  params.addParam<MeshGeneratorName>("input", "Optional input mesh to add the elements to");

  params.addRequiredParam<std::vector<Point>>("nodal_positions",
                                              "The x,y,z positions of the nodes");

  params.addRequiredParam<std::vector<dof_id_type>>("element_connectivity",
                                                    "List of nodes to use for each element");

  return params;
}

ElementGenerator::ElementGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _nodal_positions(getParam<std::vector<Point>>("nodal_positions")),
    _element_connectivity(getParam<std::vector<dof_id_type>>("element_connectivity"))
{
}

std::unique_ptr<MeshBase>
ElementGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // If there was no input mesh then let's just make a new one
  if (!mesh)
    mesh = libmesh_make_unique<ReplicatedMesh>(comm(), 2);

  // Add all the nodes
  for (auto & point : _nodal_positions)
    mesh->add_point(point);

  // Add all the elements
  for (dof_id_type i = 0; i < _element_connectivity.size(); i += 4)
  {
    auto elem = mesh->add_elem(new Quad4);

    elem->set_node(0) = mesh->node_ptr(_element_connectivity[i]);
    elem->set_node(1) = mesh->node_ptr(_element_connectivity[i + 1]);
    elem->set_node(2) = mesh->node_ptr(_element_connectivity[i + 2]);
    elem->set_node(3) = mesh->node_ptr(_element_connectivity[i + 3]);
    elem->subdomain_id() = 0;
  }

  return mesh;
}
