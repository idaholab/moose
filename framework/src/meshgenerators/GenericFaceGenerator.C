//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericFaceGenerator.h"
// #include "CastUniquePointer.h"

// #include "libmesh/replicated_mesh.h"
#include "libmesh/int_range.h"

#include "libmesh/face_generic.h"

registerMooseObject("MooseApp", GenericFaceGenerator);

InputParameters
GenericFaceGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addParam<MeshGeneratorName>("input", "Optional input mesh to add the elements to");

  params.addRequiredParam<std::vector<Point>>("nodal_positions",
                                              "The x,y,z positions of the nodes");

  params.addRequiredParam<std::vector<std::vector<dof_id_type>>>(
      "element_connectivity", "List of nodes to use for each element");

  params.addClassDescription("Generates individual elements given a list of nodal positions.");
  params.addParam<SubdomainID>("subdomain_id", 0, "Block ID for the created elements");
  return params;
}

GenericFaceGenerator::GenericFaceGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input", /* allow_invalid = */ true)),
    _nodal_positions(getParam<std::vector<Point>>("nodal_positions")),
    _element_connectivity(getParam<std::vector<std::vector<unsigned int>>>("element_connectivity")),
{
}

std::unique_ptr<MeshBase>
GenericFaceGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // If there was no input mesh then let's just make a new one
  if (!mesh)
    mesh = buildMeshBaseObject();

  mesh->set_mesh_dimension(2, mesh->mesh_dimension()));

  std::vector<Node *> nodes;
  nodes.reserve(_nodal_positions.size());

  // Add all the nodes
  for (auto & point : _nodal_positions)
    nodes.push_back(mesh->add_point(point));

  for (const auto & element : _element_connectivity)
  {
    auto n = element.size();
    auto elem = new GenericFace(n);
    mesh->add_elem(elem);

    for (const auto i : make_range(n))

      elem->set_node(i) = nodes[element[i]];
    elem->subdomain_id() = 0;
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
