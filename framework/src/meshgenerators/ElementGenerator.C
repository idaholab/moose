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

#include "MooseEnum.h"

#include "libmesh/edge_edge2.h"
#include "libmesh/edge_edge3.h"
#include "libmesh/edge_edge4.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/face_quad9.h"
#include "libmesh/face_tri3.h"
#include "libmesh/face_tri6.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex20.h"
#include "libmesh/cell_hex27.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/cell_tet10.h"
#include "libmesh/cell_prism6.h"
#include "libmesh/cell_prism15.h"
#include "libmesh/cell_prism18.h"
#include "libmesh/cell_pyramid5.h"
#include "libmesh/cell_pyramid13.h"
#include "libmesh/cell_pyramid14.h"

registerMooseObject("MooseApp", ElementGenerator);

InputParameters
ElementGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum elem_types("EDGE2 EDGE3 EDGE4 QUAD4 QUAD8 QUAD9 TRI3 TRI6 HEX8 HEX20 HEX27 TET4 TET10 "
                       "PRISM6 PRISM15 PRISM18 PYRAMID5 PYRAMID13 PYRAMID14");

  params.addParam<MeshGeneratorName>("input", "Optional input mesh to add the elements to");

  params.addRequiredParam<std::vector<Point>>("nodal_positions",
                                              "The x,y,z positions of the nodes");

  params.addRequiredParam<std::vector<dof_id_type>>("element_connectivity",
                                                    "List of nodes to use for each element");

  params.addParam<MooseEnum>(
      "elem_type", elem_types, "The type of element from libMesh to generate");

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
  if (type == "EDGE2")
  {
    Elem * elem = new Edge2;
    return elem;
  }
  if (type == "EDGE3")
  {
    Elem * elem = new Edge3;
    return elem;
  }
  if (type == "EDGE4")
  {
    Elem * elem = new Edge4;
    return elem;
  }
  if (type == "QUAD4")
  {
    Elem * elem = new Quad4;
    return elem;
  }
  if (type == "QUAD8")
  {
    Elem * elem = new Quad8;
    return elem;
  }
  if (type == "QUAD9")
  {
    Elem * elem = new Quad9;
    return elem;
  }
  if (type == "TRI3")
  {
    Elem * elem = new Tri3;
    return elem;
  }
  if (type == "TRI6")
  {
    Elem * elem = new Tri6;
    return elem;
  }
  if (type == "HEX8")
  {
    Elem * elem = new Hex8;
    return elem;
  }
  if (type == "HEX20")
  {
    Elem * elem = new Hex20;
    return elem;
  }
  if (type == "HEX27")
  {
    Elem * elem = new Hex27;
    return elem;
  }
  if (type == "TET4")
  {
    Elem * elem = new Tet4;
    return elem;
  }
  if (type == "TET10")
  {
    Elem * elem = new Tet10;
    return elem;
  }
  if (type == "PRISM6")
  {
    Elem * elem = new Prism6;
    return elem;
  }
  if (type == "PRISM15")
  {
    Elem * elem = new Prism15;
    return elem;
  }
  if (type == "PRISM18")
  {
    Elem * elem = new Prism18;
    return elem;
  }
  if (type == "PYRAMID5")
  {
    Elem * elem = new Pyramid5;
    return elem;
  }
  if (type == "PYRAMID13")
  {
    Elem * elem = new Pyramid13;
    return elem;
  }
  if (type == "PYRAMID14")
  {
    Elem * elem = new Pyramid14;
    return elem;
  }

  mooseError("This element type is not available.");
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
    elem->subdomain_id() = 0;
  }

  return dynamic_pointer_cast<MeshBase>(mesh);
}
