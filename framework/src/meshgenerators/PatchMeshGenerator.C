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
#include "libmesh/replicated_mesh.h"
#include "libmesh/point.h"
#include "libmesh/elem.h"
#include "libmesh/node.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/face_quad4.h"
#include "libmesh/face_quad8.h"
#include "libmesh/utility.h"

registerMooseObject("MooseApp", PatchMeshGenerator);

template <>
InputParameters
validParams<PatchMeshGenerator>()
{
  InputParameters params = validParams<MeshGenerator>();
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
  {
    paramError("elem_type", "Must be QUAD4 or QUAD8 for 2-dimensional meshes.");
    std::cout << "I should be here" << std::endl;
  }

  if (_dim == 2 && _zlength != 0.0)
    paramError("z_length", "Must be zero for 2-dimensional meshes");

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
  std::unique_ptr<ReplicatedMesh> mesh = libmesh_make_unique<ReplicatedMesh>(comm(), 2);
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
      {
        if (i != 3)
          node_positions.push_back({0.5 * (node_positions[i](0) + node_positions[i + 1](0)),
                                    0.5 * (node_positions[i](1) + node_positions[i + 1](1)),
                                    0.5 * (node_positions[i](2) + node_positions[i + 1](2))});
        else
          node_positions.push_back({0.5 * (node_positions[i](0) + node_positions[i - 3](0)),
                                    0.5 * (node_positions[i](1) + node_positions[i - 3](1)),
                                    0.5 * (node_positions[i](2) + node_positions[i - 3](2))});
      }

      // Exterior to interior midside node positions
      for (unsigned i = 0; i < 4; ++i)
        node_positions.push_back({0.5 * (node_positions[i](0) + node_positions[i + 4](0)),
                                  0.5 * (node_positions[i](1) + node_positions[i + 4](1)),
                                  0.5 * (node_positions[i](2) + node_positions[i + 4](2))});

      // Interior midside node positions
      for (unsigned i = 4; i < 8; ++i)
      {
        if (i != 7)
          node_positions.push_back({0.5 * (node_positions[i](0) + node_positions[i + 1](0)),
                                    0.5 * (node_positions[i](1) + node_positions[i + 1](1)),
                                    0.5 * (node_positions[i](2) + node_positions[i + 1](2))});
        else
          node_positions.push_back({0.5 * (node_positions[i](0) + node_positions[i - 3](0)),
                                    0.5 * (node_positions[i](1) + node_positions[i - 3](1)),
                                    0.5 * (node_positions[i](2) + node_positions[i - 3](2))});
      }
    }

    for (unsigned node_id = 0; node_id < num_nodes; ++node_id)
      nodes[node_id] = mesh->add_point(node_positions[node_id], node_id);

    if (_elem_type == "QUAD8")
      makeQuad8Elems(*mesh, nodes);
    else
      makeQuad4Elems(*mesh, nodes);

    boundary_info.sideset_name(0) = "bottom";
    boundary_info.sideset_name(1) = "right";
    boundary_info.sideset_name(2) = "top";
    boundary_info.sideset_name(3) = "left";

    boundary_info.nodeset_name(100) = "bottom_left";
    boundary_info.nodeset_name(101) = "bottom_right";
    boundary_info.nodeset_name(102) = "top_right";
    boundary_info.nodeset_name(103) = "top_left";
  }
  else
  {
    const std::vector<Real> x_interior_fractions{0.249, 0.826, 0.850, 0.273, 0.320, 0.677, 0.788, 0.165};
    const std::vector<Real> y_interior_fractions{0.342, 0.288, 0.649, 0.750, 0.186, 0.305, 0.693, 0.745};
    const std::vector<Real> z_interior_fractions{0.192, 0.288, 0.263, 0.230, 0.643, 0.683, 0.644, 0.702};

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
    }

    for (unsigned node_id = 0; node_id < num_nodes; ++node_id)
      nodes[node_id] = mesh->add_point(node_positions[node_id], node_id);

    if (_elem_type == "HEX20")
      makeHex20Elems(*mesh, nodes);
    else
      makeHex8Elems(*mesh, nodes);
  }

  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}

void
PatchMeshGenerator::makeQuad4Elems(ReplicatedMesh & mesh, const std::vector<Node *> & nodes)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  std::vector<std::vector<int>> elements{
      {0, 1, 5, 4}, {1, 2, 6, 5}, {2, 3, 7, 6}, {3, 0, 4, 7}, {4, 5, 6, 7}};
  for (unsigned i = 0; i < 5; ++i)
  {
    Elem * elem = mesh.add_elem(new Quad4);
    for (unsigned j = 0; j < 4; ++j)
    {
      elem->set_node(j) = nodes[elements[i][j]];
    }
    elem->subdomain_id() = i + 1;
    if (i == 4)
      break;
    else
    {
      boundary_info.add_node(i, i + 100);
      boundary_info.add_side(elem, 0, i);
    }
  }
}

void
PatchMeshGenerator::makeQuad8Elems(ReplicatedMesh & mesh, const std::vector<Node *> & nodes)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  std::vector<std::vector<int>> elements{{0, 1, 5, 4, 8, 13, 16, 12},
                                         {1, 2, 6, 5, 9, 14, 17, 13},
                                         {2, 3, 7, 6, 10, 15, 18, 14},
                                         {3, 0, 4, 7, 11, 12, 19, 15},
                                         {4, 5, 6, 7, 16, 17, 18, 19}};
  for (unsigned i = 0; i < 5; ++i)
  {
    Elem * elem = mesh.add_elem(new Quad8);
    for (unsigned j = 0; j < 8; ++j)
    {
      elem->set_node(j) = nodes[elements[i][j]];
    }
    elem->subdomain_id() = i + 1;
    if (i == 4)
      continue;
    else
    {
      boundary_info.add_node(i, i + 100);
      boundary_info.add_side(elem, 0, i);
    }
  }
}

void
PatchMeshGenerator::makeHex8Elems(ReplicatedMesh & mesh, const std::vector<Node *> & nodes)
{
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  std::vector<std::vector<int>> elements{{12, 13, 14, 15, 4, 5, 6, 7},
                                         {0, 1, 9, 8, 4, 5, 13, 12},
                                         {0, 8, 11, 3, 4, 12, 15, 7},
                                         {8, 9, 10, 11, 12, 13, 14, 15},
                                         {1, 2, 10, 9, 5, 6, 14, 13},
                                         {11, 10, 2, 3, 15, 14, 6, 7},
                                         {0, 1, 2, 3, 8, 9, 10, 11}};

  for (unsigned i = 0; i < 7; ++i)
  {
    Elem * elem = mesh.add_elem(new Hex8);
    for (unsigned j = 0; j < 8; ++j)
    {
      elem->set_node(j) = nodes[elements[i][j]];
    }
    elem->subdomain_id() = i + 1;
    if (i == 4)
      continue;
    else
    {
      // boundary_info.add_node(i, i + 100);
      if (i == 0 || i == 6)
        boundary_info.add_side(elem, 5, i);
      else
        boundary_info.add_side(elem, 4, i);
    }
  }
}

void
PatchMeshGenerator::makeHex20Elems(ReplicatedMesh & mesh, const std::vector<Node *> & nodes)
{

}
