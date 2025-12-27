//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addRequiredParam<std::vector<std::vector<dof_id_type>>>(
      "element_connectivity",
      "List of nodes to use for each element. Use a regular vector for all element types but "
      "polyhedron. For a polyhedron, use ';' to delineate the nodes for each side polygon.");
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
    _element_connectivity(getParam<std::vector<std::vector<dof_id_type>>>("element_connectivity")),
    _elem_type(getParam<MooseEnum>("elem_type"))
{
  if (_elem_type != "C0POLYHEDRON" && _element_connectivity.size() != 1)
    paramError("element_connectivity", "Must be of size 1 for all element types but polyhedra");
}

std::unique_ptr<Elem>
ElementGenerator::getElemType(const std::string & type)
{
  return Elem::build(Utility::string_to_enum<ElemType>(type));
}

std::unique_ptr<MeshBase>
ElementGenerator::generate()
{
  std::unique_ptr<MeshBase> mesh = std::move(_input);

  // If there was no input mesh then let's just make a new one
  if (!mesh)
    mesh = buildMeshBaseObject();

  // For polyhedra we need to know the side elements first
  // For polygons we need to specify the number of sides/nodes
  std::unique_ptr<Elem> elem;
  if (_elem_type != "C0POLYGON" && _elem_type != "C0POLYHEDRON")
    elem = getElemType(_elem_type);
  else if (_elem_type == "C0POLYGON")
    elem = std::make_unique<libMesh::C0Polygon>(_nodal_positions.size());

  std::vector<Node *> nodes;
  nodes.reserve(_nodal_positions.size());

  // Add all the nodes to the mesh, keep pointers to them
  for (const auto & point : _nodal_positions)
    nodes.push_back(mesh->add_point(point));

  // Set the nodes in the element
  if (_elem_type != "C0POLYHEDRON")
  {
    auto n = elem->n_nodes();
    for (const auto j : make_range(n))
      elem->set_node(j, nodes[_element_connectivity[0][j]]);
  }
  else
  {
    const auto n_sides = _element_connectivity.size();
    // Create all the polygon sides
    std::vector<std::shared_ptr<libMesh::Polygon>> sides;
    sides.reserve(n_sides);
    for (const auto i : make_range(n_sides))
    {
      auto side = std::make_shared<libMesh::C0Polygon>(_element_connectivity[i].size());
      for (const auto i_node : index_range(_element_connectivity[i]))
        side->set_node(i_node, nodes[_element_connectivity[i][i_node]]);
      sides.push_back(side);
    }

    // With the polygons we can create the polyhedron
    elem = std::make_unique<libMesh::C0Polyhedron>(sides);
  }

  // Subdomain information
  elem->subdomain_id() = getParam<SubdomainID>("subdomain_id");
  if (isParamValid("subdomain_name"))
    mesh->subdomain_name(getParam<SubdomainID>("subdomain_id")) =
        getParam<SubdomainName>("subdomain_name");

  mesh->set_mesh_dimension(std::max((unsigned int)elem->dim(), mesh->mesh_dimension()));

  if (getParam<bool>("create_sidesets"))
    for (const auto i_side : make_range(elem->n_sides()))
      mesh->get_boundary_info().add_side(elem.get(), i_side, i_side);

  mesh->add_elem(std::move(elem));
  // We just added an element
  mesh->unset_is_prepared();

  return dynamic_pointer_cast<MeshBase>(mesh);
}
