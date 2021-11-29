//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PatchMeshGenerator.h"
#include "CastUniquePointer.h"

// libMesh includes
#include "libmesh/mesh_generation.h"
#include "libmesh/unstructured_mesh.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/boundary_info.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/cell_hex8.h"
#include "libmesh/cell_hex20.h"
#include "libmesh/utility.h"

registerMooseObject("MooseApp", PatchMeshGenerator);

InputParameters
PatchMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();
  MooseEnum dims("2=2 3", "2");
  params.addParam<MooseEnum>("dim",
                             dims,
                             "The dimension of the mesh to be generated. Patch meshes are only "
                             "valid in 2 or 3 dimensions.");

  MooseEnum elem_types("QUAD4 QUAD8 HEX8 HEX20", "QUAD4");
  params.addParam<MooseEnum>("elem_type",
                             elem_types,
                             "The type of element from libMesh to "
                             "generate (default: linear element for "
                             "requested dimension)");
  params.addParam<Real>("x_length", 0.24, "Length of the domain in the x direction.");
  params.addParam<Real>("y_length", 0.12, "Length of the domain in the y direction.");
  params.addParam<Real>("z_length", 0.0, "Length of the domain in the z direction.");
  params.addParam<Real>("x_offset", 0.0, "Offset of the Cartesian origin in the x direction.");
  params.addParam<Real>("y_offset", 0.0, "Offset of the Cartesian origin in the y direction.");
  params.addParam<Real>("z_offset", 0.0, "Offset of the Cartesian origin in the z direction.");
  params.addClassDescription("Creates 2D or 3D patch meshes.");
  return params;
}

PatchMeshGenerator::PatchMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _dim(getParam<MooseEnum>("dim")),
    _elem_type(getParam<MooseEnum>("elem_type")),
    _xlength(getParam<Real>("x_length")),
    _ylength(getParam<Real>("y_length")),
    _zlength(getParam<Real>("z_length")),
    _xoffset(getParam<Real>("x_offset")),
    _yoffset(getParam<Real>("y_offset")),
    _zoffset(getParam<Real>("z_offset"))
{
  if (_xlength <= 0.0)
    paramError("x_length", "Must be greater than zero");

  if (_ylength <= 0.0)
    paramError("y_length", "Must be greater than zero");

  if (_dim == 2 && (_elem_type == "HEX8" || _elem_type == "HEX20"))
    paramError("elem_type", "Must be QUAD4 or QUAD8 for 2-dimensional meshes.");

  if (_dim == 2 && _zlength != 0.0)
    paramError("z_length", "Must be zero for 2-dimensional meshes");

  if (_dim == 3 && !parameters.isParamSetByUser("elem_type"))
    _elem_type = "HEX8";

  if (_dim == 3 && (_elem_type == "QUAD4" || _elem_type == "QUAD8"))
    paramError("elem_type", "Must be HEX8 or HEX20 for 3-dimensional meshes.");

  // If domain dimensions are not set by user for 3-dimensions a unit cube is used
  // as default as per MacNeal and Harder (1985)
  if (_dim == 3)
  {
    if (!parameters.isParamSetByUser("x_length"))
      _xlength = 1.0;
    if (!parameters.isParamSetByUser("y_length"))
      _ylength = 1.0;
    if (!parameters.isParamSetByUser("z_length"))
      _zlength = 1.0;
  }

  if (_dim == 3 && _zlength <= 0.0)
    paramError("z_length", "Must be greater than zero for 3-dimensional meshes");
}

std::unique_ptr<MeshBase>
PatchMeshGenerator::generate()
{
  auto mesh = buildMeshBaseObject();
  BoundaryInfo & boundary_info = mesh->get_boundary_info();

  unsigned num_nodes = 0;
  if (_elem_type == "QUAD4")
    num_nodes = 8;
  else if (_elem_type == "QUAD8")
    num_nodes = 20;
  else if (_elem_type == "HEX8")
    num_nodes = 16;
  else if (_elem_type == "HEX20")
    num_nodes = 48;

  std::vector<Node *> nodes(num_nodes);

  const Real xmax = _xoffset + _xlength;
  const Real ymax = _yoffset + _ylength;
  const Real zmax = _zoffset + _zlength;

  if (_dim == 2)
  {
    const std::vector<Real> x_interior_fractions{1.0 / 6.0, 3.0 / 4.0, 2.0 / 3.0, 1.0 / 3.0};
    const std::vector<Real> y_interior_fractions{1.0 / 6.0, 1.0 / 4.0, 2.0 / 3.0, 2.0 / 3.0};

    // Exterior node positions
    std::vector<Point> node_positions{{_xoffset, _yoffset, _zoffset},
                                      {xmax, _yoffset, _zoffset},
                                      {xmax, ymax, _zoffset},
                                      {_xoffset, ymax, _zoffset}};
    // Interior node positions
    for (unsigned i = 0; i < x_interior_fractions.size(); ++i)
      node_positions.push_back({_xoffset + x_interior_fractions[i] * _xlength,
                                _yoffset + y_interior_fractions[i] * _ylength,
                                _zoffset});

    if (_elem_type == "QUAD8")
    {
      // Exterior midside node positions
      for (unsigned i = 0; i < 4; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[(i + 1) % 4]));

      // Exterior to interior midside node positions
      for (unsigned i = 0; i < 4; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[i + 4]));

      // Interior midside node positions
      for (unsigned i = 4; i < 8; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[(i + 1) % 4 + 4]));
    }

    for (unsigned node_id = 0; node_id < num_nodes; ++node_id)
      nodes[node_id] = mesh->add_point(node_positions[node_id], node_id);

    if (_elem_type == "QUAD4")
      makeQuad4Elems(*mesh, nodes);
    else
      makeQuad8Elems(*mesh, nodes);

    boundary_info.sideset_name(1) = "bottom";
    boundary_info.sideset_name(2) = "right";
    boundary_info.sideset_name(3) = "top";
    boundary_info.sideset_name(4) = "left";

    boundary_info.nodeset_name(100) = "bottom_left";
    boundary_info.nodeset_name(101) = "bottom_right";
    boundary_info.nodeset_name(102) = "top_right";
    boundary_info.nodeset_name(103) = "top_left";
  }
  else
  {
    const std::vector<Real> x_interior_fractions{
        0.249, 0.826, 0.850, 0.273, 0.320, 0.677, 0.788, 0.165};
    const std::vector<Real> y_interior_fractions{
        0.342, 0.288, 0.649, 0.750, 0.186, 0.305, 0.693, 0.745};
    const std::vector<Real> z_interior_fractions{
        0.192, 0.288, 0.263, 0.230, 0.643, 0.683, 0.644, 0.702};

    // Exterior node positions
    std::vector<Point> node_positions{{_xoffset, _yoffset, _zoffset},
                                      {xmax, _yoffset, _zoffset},
                                      {xmax, ymax, _zoffset},
                                      {_xoffset, ymax, _zoffset},
                                      {_xoffset, _yoffset, zmax},
                                      {xmax, _yoffset, zmax},
                                      {xmax, ymax, zmax},
                                      {_xoffset, ymax, zmax}};

    // Interior node positions
    for (unsigned i = 0; i < x_interior_fractions.size(); ++i)
      node_positions.push_back({_xoffset + x_interior_fractions[i] * _xlength,
                                _yoffset + y_interior_fractions[i] * _ylength,
                                _zoffset + z_interior_fractions[i] * _zlength});

    if (_elem_type == "HEX20")
    {
      // Midside Nodes

      // Four on back face
      for (unsigned i = 0; i < 4; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[(i + 1) % 4]));

      // Four on front face
      for (unsigned i = 4; i < 8; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[(i + 1) % 4 + 4]));

      // Four on remaining exterior edges
      for (unsigned i = 0; i < 4; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[i + 4]));

      // Four on interior hex back face
      for (unsigned i = 8; i < 12; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[(i + 1) % 4 + 8]));

      // Four on interior hex front face
      for (unsigned i = 12; i < 16; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[(i + 1) % 4 + 12]));

      // Four on remaining interior hex edges
      for (unsigned i = 8; i < 12; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[i + 4]));

      // Eight on remaining interior edges
      for (unsigned i = 0; i < 8; ++i)
        node_positions.push_back(0.5 * (node_positions[i] + node_positions[i + 8]));
    }

    for (unsigned node_id = 0; node_id < num_nodes; ++node_id)
      nodes[node_id] = mesh->add_point(node_positions[node_id], node_id);

    if (_elem_type == "HEX8")
      makeHex8Elems(*mesh, nodes);
    else
      makeHex20Elems(*mesh, nodes);

    boundary_info.sideset_name(1) = "front";
    boundary_info.sideset_name(2) = "bottom";
    boundary_info.sideset_name(3) = "left";
    boundary_info.sideset_name(4) = "right";
    boundary_info.sideset_name(5) = "top";
    boundary_info.sideset_name(6) = "back";

    boundary_info.nodeset_name(100) = "bottom_back_left";
    boundary_info.nodeset_name(101) = "bottom_back_right";
    boundary_info.nodeset_name(102) = "top_back_right";
    boundary_info.nodeset_name(103) = "top_back_left";
    boundary_info.nodeset_name(104) = "bottom_front_left";
    boundary_info.nodeset_name(105) = "bottom_front_right";
    boundary_info.nodeset_name(106) = "top_front_right";
    boundary_info.nodeset_name(107) = "top_front_left";
  }

  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
PatchMeshGenerator::makeQuad4Elems(MeshBase & mesh, const std::vector<Node *> & nodes)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  std::vector<std::vector<int>> element_connectivity{
      {0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}, {4, 5, 6, 7}};
  for (unsigned i = 0; i < 5; ++i)
  {
    Elem * elem = mesh.add_elem(new Quad4);
    for (unsigned j = 0; j < 4; ++j)
      elem->set_node(j) = nodes[element_connectivity[i][j]];

    elem->subdomain_id() = i + 1;

    if (i < 4)
    {
      boundary_info.add_node(i, i + 100);
      boundary_info.add_side(elem, 0, i + 1);
    }
  }
}

void
PatchMeshGenerator::makeQuad8Elems(MeshBase & mesh, const std::vector<Node *> & nodes)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  std::vector<std::vector<int>> element_connectivity{{0, 1, 5, 4, 8, 13, 16, 12},
                                                     {1, 2, 6, 5, 9, 14, 17, 13},
                                                     {2, 3, 7, 6, 10, 15, 18, 14},
                                                     {3, 0, 4, 7, 11, 12, 19, 15},
                                                     {4, 5, 6, 7, 16, 17, 18, 19}};
  for (unsigned i = 0; i < 5; ++i)
  {
    Elem * elem = mesh.add_elem(new Quad8);
    for (unsigned j = 0; j < 8; ++j)
      elem->set_node(j) = nodes[element_connectivity[i][j]];

    elem->subdomain_id() = i + 1;

    if (i < 4)
    {
      boundary_info.add_node(i, i + 100);
      boundary_info.add_side(elem, 0, i + 1);
    }
  }
}

void
PatchMeshGenerator::makeHex8Elems(MeshBase & mesh, const std::vector<Node *> & nodes)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  std::vector<std::vector<int>> element_connectivity{{12, 13, 14, 15, 4, 5, 6, 7},
                                                     {8, 9, 13, 12, 0, 1, 5, 4},
                                                     {8, 12, 15, 11, 0, 4, 7, 3},
                                                     {9, 10, 14, 13, 1, 2, 6, 5},
                                                     {10, 11, 15, 14, 2, 3, 7, 6},
                                                     {8, 11, 10, 9, 0, 3, 2, 1},
                                                     {8, 9, 10, 11, 12, 13, 14, 15}};

  for (unsigned i = 0; i < 7; ++i)
  {
    Elem * elem = mesh.add_elem(new Hex8);
    for (unsigned j = 0; j < 8; ++j)
      elem->set_node(j) = nodes[element_connectivity[i][j]];

    elem->subdomain_id() = i + 1;
    if (i < 6)
      boundary_info.add_side(elem, 5, i + 1);
  }
  for (unsigned i = 0; i < 8; ++i)
    boundary_info.add_node(i, i + 100);
}

void
PatchMeshGenerator::makeHex20Elems(MeshBase & mesh, const std::vector<Node *> & nodes)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  std::vector<std::vector<int>> element_connectivity{
      {12, 13, 14, 15, 4, 5, 6, 7, 32, 33, 34, 35, 44, 45, 46, 47, 20, 21, 22, 23},
      {8, 9, 13, 12, 0, 1, 5, 4, 28, 37, 32, 36, 40, 41, 45, 44, 16, 25, 20, 24},
      {8, 12, 15, 11, 0, 4, 7, 3, 36, 35, 39, 31, 40, 44, 47, 43, 24, 23, 27, 19},
      {9, 10, 14, 13, 1, 2, 6, 5, 29, 38, 33, 37, 41, 42, 46, 45, 17, 26, 21, 25},
      {10, 11, 15, 14, 2, 3, 7, 6, 30, 39, 34, 38, 42, 43, 47, 46, 18, 27, 22, 26},
      {8, 11, 10, 9, 0, 3, 2, 1, 31, 30, 29, 28, 40, 43, 42, 41, 19, 18, 17, 16},
      {8, 9, 10, 11, 12, 13, 14, 15, 28, 29, 30, 31, 36, 37, 38, 39, 32, 33, 34, 35}};

  for (unsigned i = 0; i < 7; ++i)
  {
    Elem * elem = mesh.add_elem(new Hex20);
    for (unsigned j = 0; j < 20; ++j)
      elem->set_node(j) = nodes[element_connectivity[i][j]];

    elem->subdomain_id() = i + 1;
    if (i < 6)
      boundary_info.add_side(elem, 5, i + 1);
  }
  for (unsigned i = 0; i < 8; ++i)
    boundary_info.add_node(i, i + 100);
}
