//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementGenerator.h"
#include "CastUniquePointer.h"

#include "libmesh/replicated_mesh.h"
#include "libmesh/string_to_enum.h"

#include "MooseEnum.h"

registerMooseObject("MooseApp", ElementGenerator);

InputParameters
ElementGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum elem_types(LIST_GEOM_ELEM); // no default

  params.addParam<MeshGeneratorName>("input", "Optional input mesh to add the elements to");

  // Element shape and location
  params.addRequiredParam<std::vector<Point>>("nodal_positions",
                                              "The x,y,z positions of the nodes");
  params.addRequiredParam<std::vector<dof_id_type>>("element_connectivity",
                                                    "List of nodes to use for each element");
  params.addRequiredParam<MooseEnum>(
      "elem_type", elem_types, "The type of element from libMesh to generate");

  // Subdomain
  params.addParam<SubdomainName>("subdomain_name", "Subdomain name");
  params.addParam<SubdomainID>("subdomain_id", 0, "Subdomain id");
  // Sidesets
  params.addParam<bool>("create_sidesets",
                        false,
                        "Create separate sidesets for each side. "
                        "The side index is used as the boundary ID for each sideset.");

  params.addClassDescription("Generates individual elements given a list of nodal positions.");

  return params;
}

ElementGenerator::ElementGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input", /* allow_invalid = */ true)),
    _nodal_positions(getParam<std::vector<Point>>("nodal_positions")),
    _element_connectivity(getParam<std::vector<dof_id_type>>("element_connectivity")),
    _elem_type(getParam<MooseEnum>("elem_type"))
{
}

Elem *
ElementGenerator::getElemType(const std::string & type)
{
  return Elem::build(Utility::string_to_enum<ElemType>(type)).release();
}

std::unique_ptr<MeshBase>
ElementGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // If there was no input mesh then let's just make a new one
  if (!mesh)
    mesh = buildMeshBaseObject();

  MooseEnum elem_type_enum = getParam<MooseEnum>("elem_type");
  auto elem = getElemType(elem_type_enum);
  elem->subdomain_id() = getParam<SubdomainID>("subdomain_id");
  if (isParamValid("subdomain_name"))
    mesh->subdomain_name(getParam<SubdomainID>("subdomain_id")) =
        getParam<SubdomainName>("subdomain_name");

  mesh->set_mesh_dimension(std::max((unsigned int)elem->dim(), mesh->mesh_dimension()));

  std::vector<Node *> nodes;

  nodes.reserve(_nodal_positions.size());

  // Add all the nodes
  for (auto & point : _nodal_positions)
    nodes.push_back(mesh->add_point(point));

  mesh->add_elem(elem);

  auto n = elem->n_nodes();

  for (dof_id_type i = 0; i < _element_connectivity.size(); i += n)
  {
    for (unsigned int j = 0; j < n; j++)
    {
      elem->set_node(j) = nodes[_element_connectivity[j + i]];
    }
  }

  if (getParam<bool>("create_sidesets"))
    for (const auto i_side : make_range(elem->n_sides()))
      mesh->get_boundary_info().add_side(elem, i_side, i_side);

  return dynamic_pointer_cast<MeshBase>(mesh);
}
