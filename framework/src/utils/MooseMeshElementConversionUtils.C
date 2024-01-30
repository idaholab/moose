//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseMeshElementConversionUtils.h"

#include "libmesh/elem.h"
#include "libmesh/enum_order.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_base.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/utility.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/face_tri3.h"

using namespace libMesh;

namespace MooseMeshElementConversionUtils
{
void
hexElemSplitter(ReplicatedMesh & mesh,
                const dof_id_type elem_id,
                std::vector<dof_id_type> & converted_elems_ids)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(6);
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
  {
    if (std::get<0>(bdry_side_list[i]) == elem_id)
    {
      elem_side_list[std::get<1>(bdry_side_list[i])].push_back(std::get<2>(bdry_side_list[i]));
    }
  }
  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  std::vector<std::vector<unsigned int>> opt_option;
  std::vector<Node *> elem_node_list = {mesh.elem_ptr(elem_id)->node_ptr(0),
                                        mesh.elem_ptr(elem_id)->node_ptr(1),
                                        mesh.elem_ptr(elem_id)->node_ptr(2),
                                        mesh.elem_ptr(elem_id)->node_ptr(3),
                                        mesh.elem_ptr(elem_id)->node_ptr(4),
                                        mesh.elem_ptr(elem_id)->node_ptr(5),
                                        mesh.elem_ptr(elem_id)->node_ptr(6),
                                        mesh.elem_ptr(elem_id)->node_ptr(7)};
  std::vector<std::vector<unsigned int>> rotated_tet_face_indices;
  auto optimized_node_list = hexNodesToTetNodesDeterminer(elem_node_list, rotated_tet_face_indices);

  std::vector<Elem *> elems_Tet4;
  for (unsigned int i = 0; i < optimized_node_list.size(); i++)
  {
    elems_Tet4.push_back(mesh.add_elem(new Tet4));
    elems_Tet4.back()->set_node(0) = optimized_node_list[i][0];
    elems_Tet4.back()->set_node(1) = optimized_node_list[i][1];
    elems_Tet4.back()->set_node(2) = optimized_node_list[i][2];
    elems_Tet4.back()->set_node(3) = optimized_node_list[i][3];
    elems_Tet4.back()->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id();
    converted_elems_ids.push_back(elems_Tet4.back()->id());

    for (unsigned int j = 0; j < 4; j++)
    {
      if (rotated_tet_face_indices[i][j] < 6)
      {
        for (const auto & side_info : elem_side_list[rotated_tet_face_indices[i][j]])
          boundary_info.add_side(elems_Tet4.back(), j, side_info);
      }
    }
  }

  // Retain element extra integers
  for (unsigned int i = 0; i < 2; i++)
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    {
      elems_Tet4[i]->set_extra_integer(j, exist_extra_ids[j]);
    }
}

void
prismElemSplitter(ReplicatedMesh & mesh,
                  const dof_id_type elem_id,
                  std::vector<dof_id_type> & converted_elems_ids)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(5);
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
  {
    if (std::get<0>(bdry_side_list[i]) == elem_id)
    {
      elem_side_list[std::get<1>(bdry_side_list[i])].push_back(std::get<2>(bdry_side_list[i]));
    }
  }

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);

  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  std::vector<Node *> elem_node_list = {mesh.elem_ptr(elem_id)->node_ptr(0),
                                        mesh.elem_ptr(elem_id)->node_ptr(1),
                                        mesh.elem_ptr(elem_id)->node_ptr(2),
                                        mesh.elem_ptr(elem_id)->node_ptr(3),
                                        mesh.elem_ptr(elem_id)->node_ptr(4),
                                        mesh.elem_ptr(elem_id)->node_ptr(5)};
  std::vector<std::vector<unsigned int>> rotated_tet_face_indices;
  auto optimized_node_list =
      prismNodesToTetNodesDeterminer(elem_node_list, rotated_tet_face_indices);

  std::vector<Elem *> elems_Tet4;
  for (unsigned int i = 0; i < optimized_node_list.size(); i++)
  {
    elems_Tet4.push_back(mesh.add_elem(new Tet4));
    elems_Tet4.back()->set_node(0) = optimized_node_list[i][0];
    elems_Tet4.back()->set_node(1) = optimized_node_list[i][1];
    elems_Tet4.back()->set_node(2) = optimized_node_list[i][2];
    elems_Tet4.back()->set_node(3) = optimized_node_list[i][3];
    elems_Tet4.back()->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id();
    converted_elems_ids.push_back(elems_Tet4.back()->id());

    for (unsigned int j = 0; j < 4; j++)
    {
      if (rotated_tet_face_indices[i][j] < 5)
      {
        for (const auto & side_info : elem_side_list[rotated_tet_face_indices[i][j]])
          boundary_info.add_side(elems_Tet4.back(), j, side_info);
      }
    }
  }

  // Retain element extra integers
  for (unsigned int i = 0; i < 3; i++)
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    {
      elems_Tet4[i]->set_extra_integer(j, exist_extra_ids[j]);
    }
}

void
pyramidElemSplitter(ReplicatedMesh & mesh,
                    const dof_id_type elem_id,
                    std::vector<dof_id_type> & converted_elems_ids)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  auto bdry_side_list = boundary_info.build_side_list();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(5);
  for (unsigned int i = 0; i < bdry_side_list.size(); i++)
  {
    if (std::get<0>(bdry_side_list[i]) == elem_id)
    {
      elem_side_list[std::get<1>(bdry_side_list[i])].push_back(std::get<2>(bdry_side_list[i]));
    }
  }

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  std::vector<Node *> elem_node_list = {mesh.elem_ptr(elem_id)->node_ptr(0),
                                        mesh.elem_ptr(elem_id)->node_ptr(1),
                                        mesh.elem_ptr(elem_id)->node_ptr(2),
                                        mesh.elem_ptr(elem_id)->node_ptr(3),
                                        mesh.elem_ptr(elem_id)->node_ptr(4)};
  std::vector<std::vector<unsigned int>> rotated_tet_face_indices;
  auto optimized_node_list =
      pyramidNodesToTetNodesDeterminer(elem_node_list, rotated_tet_face_indices);

  std::vector<Elem *> elems_Tet4;
  for (unsigned int i = 0; i < optimized_node_list.size(); i++)
  {
    elems_Tet4.push_back(mesh.add_elem(new Tet4));
    elems_Tet4.back()->set_node(0) = optimized_node_list[i][0];
    elems_Tet4.back()->set_node(1) = optimized_node_list[i][1];
    elems_Tet4.back()->set_node(2) = optimized_node_list[i][2];
    elems_Tet4.back()->set_node(3) = optimized_node_list[i][3];
    elems_Tet4.back()->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id();
    converted_elems_ids.push_back(elems_Tet4.back()->id());

    for (unsigned int j = 0; j < 4; j++)
    {
      if (rotated_tet_face_indices[i][j] < 5)
      {
        for (const auto & side_info : elem_side_list[rotated_tet_face_indices[i][j]])
          boundary_info.add_side(elems_Tet4.back(), j, side_info);
      }
    }
  }

  // Retain element extra integers
  for (unsigned int i = 0; i < 2; i++)
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    {
      elems_Tet4[i]->set_extra_integer(j, exist_extra_ids[j]);
    }
}

std::vector<unsigned int>
neighborNodeIndicesHEX8(unsigned int min_id_index)
{
  const std::vector<std::vector<unsigned int>> preset_indices = {
      {1, 3, 4}, {0, 2, 5}, {3, 1, 6}, {2, 0, 7}, {5, 7, 0}, {4, 6, 1}, {7, 5, 2}, {6, 4, 3}};
  if (min_id_index > 7)
    mooseError("The input node index is out of range.");
  else
    return preset_indices[min_id_index];
}

std::vector<std::vector<Node *>>
hexNodesToTetNodesDeterminer(std::vector<Node *> & hex_nodes,
                             std::vector<std::vector<unsigned int>> & rotated_tet_face_indices)
{
  // Find the node with the minimum id
  std::vector<dof_id_type> node_ids(8);
  for (unsigned int i = 0; i < 8; i++)
    node_ids[i] = hex_nodes[i]->id();

  const unsigned int min_node_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));
  const auto neighbor_node_indices = neighborNodeIndicesHEX8(min_node_id_index);

  const auto neighbor_node_ids = {node_ids[neighbor_node_indices[0]],
                                  node_ids[neighbor_node_indices[1]],
                                  node_ids[neighbor_node_indices[2]]};
  const unsigned int sec_min_pos =
      std::distance(std::begin(neighbor_node_ids),
                    std::min_element(std::begin(neighbor_node_ids), std::end(neighbor_node_ids)));

  std::vector<unsigned int> face_rotation;
  const auto rotated_indices = nodeRotationHEX8(min_node_id_index, sec_min_pos, face_rotation);
  std::vector<Node *> rotated_hex_nodes;
  for (unsigned int i = 0; i < 8; i++)
    rotated_hex_nodes.push_back(hex_nodes[rotated_indices[i]]);

  const auto diagonal_directions = quadFaceDiagonalDirectionsHex(rotated_hex_nodes);

  std::vector<std::vector<unsigned int>> tet_face_indices;
  const auto tet_nodes_set = tetNodesForHex(diagonal_directions, tet_face_indices);
  for (const auto & tet_face_index : tet_face_indices)
  {
    rotated_tet_face_indices.push_back(std::vector<unsigned int>());
    for (const auto & face_index : tet_face_index)
    {
      if (face_index < 6)
        rotated_tet_face_indices.back().push_back(face_rotation[face_index]);
      else
        rotated_tet_face_indices.back().push_back(6);
    }
  }

  std::vector<std::vector<Node *>> tet_nodes_list;
  for (const auto & tet_nodes : tet_nodes_set)
  {
    tet_nodes_list.push_back(std::vector<Node *>());
    for (const auto & tet_node : tet_nodes)
      tet_nodes_list.back().push_back(rotated_hex_nodes[tet_node]);
  }

  return tet_nodes_list;
}

std::vector<bool>
quadFaceDiagonalDirectionsHex(const std::vector<Node *> & hex_nodes)
{
  // Bottom/Top; Front/Back; Right/Left
  const std::vector<std::vector<unsigned int>> face_indices = {
      {0, 1, 2, 3}, {4, 5, 6, 7}, {0, 1, 5, 4}, {2, 3, 7, 6}, {1, 2, 6, 5}, {3, 0, 4, 7}};
  std::vector<bool> diagonal_directions;
  for (const auto & face_index : face_indices)
  {
    std::vector<Node *> quad_nodes = {hex_nodes[face_index[0]],
                                      hex_nodes[face_index[1]],
                                      hex_nodes[face_index[2]],
                                      hex_nodes[face_index[3]]};
    diagonal_directions.push_back(quadFaceDiagonalDirection(quad_nodes));
  }
  return diagonal_directions;
}

bool
quadFaceDiagonalDirection(const std::vector<Node *> & quad_nodes)
{
  const std::vector<dof_id_type> node_ids = {
      quad_nodes[0]->id(), quad_nodes[1]->id(), quad_nodes[2]->id(), quad_nodes[3]->id()};
  const unsigned int min_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));
  if (min_id_index == 0 || min_id_index == 2)
    return true;
  else
    return false;
}

std::vector<std::vector<unsigned int>>
tetNodesForHex(const std::vector<bool> diagonal_directions,
               std::vector<std::vector<unsigned int>> & tet_face_indices)
{
  const std::vector<std::vector<bool>> possible_inputs = {{true, true, true, true, true, false},
                                                          {true, true, true, true, false, false},
                                                          {true, true, true, false, true, false},
                                                          {true, false, true, true, true, false},
                                                          {true, false, true, true, false, false},
                                                          {true, false, true, false, true, false},
                                                          {true, false, true, false, false, false}};

  const unsigned int input_index = std::distance(
      std::begin(possible_inputs),
      std::find(std::begin(possible_inputs), std::end(possible_inputs), diagonal_directions));

  switch (input_index)
  {
    case 0:
      tet_face_indices = {
          {0, 6, 2, 6}, {1, 6, 2, 6}, {1, 6, 5, 6}, {0, 6, 3, 4}, {6, 6, 3, 6}, {6, 4, 5, 6}};
      return {{0, 1, 2, 6}, {0, 5, 1, 6}, {0, 4, 5, 6}, {0, 2, 3, 7}, {0, 6, 2, 7}, {0, 4, 6, 7}};
    case 1:
      tet_face_indices = {
          {0, 1, 2, 6}, {6, 6, 2, 6}, {6, 6, 5, 1}, {0, 6, 3, 4}, {6, 6, 3, 6}, {6, 4, 5, 6}};
      return {{0, 1, 2, 5}, {0, 2, 6, 5}, {0, 6, 4, 5}, {0, 2, 3, 7}, {0, 6, 2, 7}, {0, 4, 6, 7}};
    case 2:
      tet_face_indices = {
          {0, 6, 2, 6}, {1, 6, 2, 6}, {1, 6, 5, 6}, {4, 6, 5, 6}, {4, 6, 3, 6}, {0, 6, 3, 6}};
      return {{0, 1, 2, 6}, {0, 5, 1, 6}, {0, 4, 5, 6}, {0, 7, 4, 6}, {0, 3, 7, 6}, {0, 2, 3, 6}};
    case 3:
      tet_face_indices = {
          {4, 6, 5, 1}, {6, 6, 5, 6}, {6, 1, 2, 6}, {4, 0, 3, 6}, {6, 6, 3, 6}, {6, 6, 2, 0}};
      return {{0, 7, 4, 5}, {0, 6, 7, 5}, {0, 1, 6, 5}, {0, 3, 7, 2}, {0, 7, 6, 2}, {0, 6, 1, 2}};
    case 4:
      tet_face_indices = {{0, 1, 2, 6}, {0, 6, 3, 4}, {5, 4, 6, 1}, {5, 6, 3, 2}, {6, 6, 6, 6}};
      return {{0, 1, 2, 5}, {0, 2, 3, 7}, {4, 7, 5, 0}, {5, 7, 6, 2}, {0, 2, 7, 5}};
    case 5:
      tet_face_indices = {
          {4, 6, 5, 1}, {6, 6, 5, 6}, {6, 1, 2, 6}, {2, 6, 6, 0}, {3, 6, 6, 0}, {3, 6, 6, 4}};
      return {{0, 7, 4, 5}, {0, 6, 7, 5}, {0, 1, 6, 5}, {1, 6, 2, 0}, {2, 6, 3, 0}, {3, 6, 7, 0}};
    case 6:
      tet_face_indices = {
          {1, 4, 5, 6}, {6, 6, 5, 6}, {6, 6, 3, 4}, {1, 6, 2, 0}, {6, 6, 2, 6}, {6, 0, 3, 6}};
      return {{0, 4, 5, 7}, {0, 5, 6, 7}, {0, 6, 3, 7}, {0, 5, 1, 2}, {0, 6, 5, 2}, {0, 3, 6, 2}};
    default:
      mooseError("Unexpected input.");
  }
}

std::vector<unsigned int>
nodeRotationHEX8(unsigned int min_id_index,
                 unsigned int sec_min_pos,
                 std::vector<unsigned int> & face_rotation)
{
  const std::vector<std::vector<std::vector<unsigned int>>> preset_indices = {
      {{0, 1, 2, 3, 4, 5, 6, 7}, {0, 3, 7, 4, 1, 2, 6, 5}, {0, 4, 5, 1, 3, 7, 6, 2}},
      {{1, 0, 4, 5, 2, 3, 7, 6}, {1, 2, 3, 0, 5, 6, 7, 4}, {1, 5, 6, 2, 0, 4, 7, 3}},
      {{2, 3, 0, 1, 6, 7, 4, 5}, {2, 1, 5, 6, 3, 0, 4, 7}, {2, 6, 7, 3, 1, 5, 4, 0}},
      {{3, 2, 6, 7, 0, 1, 5, 4}, {3, 0, 1, 2, 7, 4, 5, 6}, {3, 7, 4, 0, 2, 6, 5, 1}},
      {{4, 5, 1, 0, 7, 6, 2, 3}, {4, 7, 6, 5, 0, 3, 2, 1}, {4, 0, 3, 7, 5, 1, 2, 6}},
      {{5, 4, 7, 6, 1, 0, 3, 2}, {5, 6, 2, 1, 4, 7, 3, 0}, {5, 1, 0, 4, 6, 2, 3, 7}},
      {{6, 7, 3, 2, 5, 4, 0, 1}, {6, 5, 4, 7, 2, 1, 0, 3}, {6, 2, 1, 5, 7, 3, 0, 4}},
      {{7, 6, 5, 4, 3, 2, 1, 0}, {7, 4, 0, 3, 6, 5, 1, 2}, {7, 3, 2, 6, 4, 0, 1, 5}}};

  const std::vector<std::vector<std::vector<unsigned int>>> preset_face_indices = {
      {{0, 1, 2, 3, 4, 5}, {4, 0, 3, 5, 1, 2}, {1, 4, 5, 2, 0, 3}},
      {{1, 0, 4, 5, 2, 3}, {0, 2, 3, 4, 1, 5}, {2, 1, 5, 3, 0, 4}},
      {{0, 3, 4, 1, 2, 5}, {2, 0, 1, 5, 3, 4}, {3, 2, 5, 4, 0, 1}},
      {{3, 0, 2, 5, 4, 5}, {0, 4, 1, 2, 3, 5}, {4, 3, 5, 1, 0, 2}},
      {{2, 5, 2, 0, 4, 3}, {5, 4, 3, 2, 2, 0}, {4, 1, 0, 3, 5, 2}},
      {{5, 1, 4, 3, 5, 0}, {2, 5, 3, 0, 1, 4}, {1, 2, 0, 4, 5, 3}},
      {{3, 5, 4, 0, 2, 1}, {5, 2, 1, 4, 3, 0}, {2, 3, 0, 1, 5, 4}},
      {{5, 3, 2, 1, 4, 0}, {4, 5, 1, 0, 3, 2}, {3, 4, 0, 2, 5, 1}}};

  if (min_id_index > 7 || sec_min_pos > 2)
    mooseError("The input node index is out of range.");
  else
  {
    // index: new face index; value: old face index
    face_rotation = preset_face_indices[min_id_index][sec_min_pos];
    return preset_indices[min_id_index][sec_min_pos];
  }
}

std::vector<unsigned int>
nodeRotationPRISM6(unsigned int min_id_index, std::vector<unsigned int> & face_rotation)
{
  const std::vector<std::vector<unsigned int>> preset_indices = {{0, 1, 2, 3, 4, 5},
                                                                 {1, 2, 0, 4, 5, 3},
                                                                 {2, 0, 1, 5, 3, 4},
                                                                 {3, 5, 4, 0, 2, 1},
                                                                 {4, 3, 5, 1, 0, 2},
                                                                 {5, 4, 3, 2, 1, 0}};

  const std::vector<std::vector<unsigned int>> preset_face_indices = {{0, 1, 2, 3, 4},
                                                                      {0, 2, 3, 1, 4},
                                                                      {0, 3, 1, 2, 4},
                                                                      {4, 3, 2, 1, 0},
                                                                      {4, 1, 3, 2, 0},
                                                                      {4, 2, 1, 3, 0}};

  if (min_id_index > 5)
    mooseError("The input node index is out of range.");
  else
  {
    // index: new face index; value: old face index
    face_rotation = preset_face_indices[min_id_index];
    return preset_indices[min_id_index];
  }
}

std::vector<std::vector<Node *>>
prismNodesToTetNodesDeterminer(std::vector<Node *> & prism_nodes,
                               std::vector<std::vector<unsigned int>> & rotated_tet_face_indices)
{
  // Find the node with the minimum id
  std::vector<dof_id_type> node_ids(6);
  for (unsigned int i = 0; i < 6; i++)
    node_ids[i] = prism_nodes[i]->id();

  const unsigned int min_node_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));

  std::vector<unsigned int> face_rotation;
  const auto rotated_indices = nodeRotationPRISM6(min_node_id_index, face_rotation);
  std::vector<Node *> rotated_prism_nodes;
  for (unsigned int i = 0; i < 6; i++)
    rotated_prism_nodes.push_back(prism_nodes[rotated_indices[i]]);

  std::vector<Node *> key_quad_nodes = {rotated_prism_nodes[1],
                                        rotated_prism_nodes[2],
                                        rotated_prism_nodes[5],
                                        rotated_prism_nodes[4]};

  const bool diagonal_direction = quadFaceDiagonalDirection(key_quad_nodes);

  std::vector<std::vector<unsigned int>> tet_face_indices;
  const auto tet_nodes_set = tetNodesForPrism(diagonal_direction, tet_face_indices);
  for (const auto & tet_face_index : tet_face_indices)
  {
    rotated_tet_face_indices.push_back(std::vector<unsigned int>());
    for (const auto & face_index : tet_face_index)
    {
      if (face_index < 5)
        rotated_tet_face_indices.back().push_back(face_rotation[face_index]);
      else
        rotated_tet_face_indices.back().push_back(5);
    }
  }

  std::vector<std::vector<Node *>> tet_nodes_list;
  for (const auto & tet_nodes : tet_nodes_set)
  {
    tet_nodes_list.push_back(std::vector<Node *>());
    for (const auto & tet_node : tet_nodes)
      tet_nodes_list.back().push_back(rotated_prism_nodes[tet_node]);
  }

  return tet_nodes_list;
}

std::vector<std::vector<unsigned int>>
tetNodesForPrism(const bool diagonal_direction,
                 std::vector<std::vector<unsigned int>> & tet_face_indices)
{

  if (diagonal_direction)
  {
    tet_face_indices = {{4, 3, 5, 1}, {2, 1, 5, 5}, {2, 5, 3, 0}};
    return {{3, 5, 4, 0}, {1, 4, 5, 0}, {1, 5, 2, 0}};
  }
  else
  {
    tet_face_indices = {{4, 3, 5, 1}, {2, 1, 5, 0}, {2, 5, 5, 3}};
    return {{3, 5, 4, 0}, {1, 4, 2, 0}, {2, 4, 5, 0}};
  }
}

std::vector<unsigned int>
nodeRotationPYRAMIND5(unsigned int min_id_index, std::vector<unsigned int> & face_rotation)
{
  const std::vector<std::vector<unsigned int>> preset_indices = {
      {0, 1, 2, 3, 4}, {1, 2, 3, 0, 4}, {2, 3, 0, 1, 4}, {3, 0, 1, 2, 4}};

  const std::vector<std::vector<unsigned int>> preset_face_indices = {
      {0, 1, 2, 3, 4}, {1, 2, 3, 0, 4}, {2, 3, 0, 1, 4}, {3, 0, 1, 2, 4}};

  if (min_id_index > 3)
    mooseError("The input node index is out of range.");
  else
  {
    // index: new face index; value: old face index
    face_rotation = preset_face_indices[min_id_index];
    return preset_indices[min_id_index];
  }
}

std::vector<std::vector<Node *>>
pyramidNodesToTetNodesDeterminer(std::vector<Node *> & pyramid_nodes,
                                 std::vector<std::vector<unsigned int>> & rotated_tet_face_indices)
{
  // Find the node with the minimum id, ignoring the top node
  std::vector<dof_id_type> node_ids(4);
  for (unsigned int i = 0; i < 4; i++)
    node_ids[i] = pyramid_nodes[i]->id();

  const unsigned int min_node_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));

  std::vector<unsigned int> face_rotation;
  const auto rotated_indices = nodeRotationPYRAMIND5(min_node_id_index, face_rotation);
  std::vector<Node *> rotated_pyramid_nodes;
  for (unsigned int i = 0; i < 5; i++)
    rotated_pyramid_nodes.push_back(pyramid_nodes[rotated_indices[i]]);

  const std::vector<std::vector<unsigned int>> tet_nodes_set = {{0, 1, 2, 4}, {0, 2, 3, 4}};
  const std::vector<std::vector<unsigned int>> tet_face_indices = {{4, 0, 1, 5}, {4, 5, 2, 3}};

  for (const auto & tet_face_index : tet_face_indices)
  {
    rotated_tet_face_indices.push_back(std::vector<unsigned int>());
    for (const auto & face_index : tet_face_index)
    {
      if (face_index < 5)
        rotated_tet_face_indices.back().push_back(face_rotation[face_index]);
      else
        rotated_tet_face_indices.back().push_back(5);
    }
  }

  std::vector<std::vector<Node *>> tet_nodes_list;
  for (const auto & tet_nodes : tet_nodes_set)
  {
    tet_nodes_list.push_back(std::vector<Node *>());
    for (const auto & tet_node : tet_nodes)
      tet_nodes_list.back().push_back(rotated_pyramid_nodes[tet_node]);
  }

  return tet_nodes_list;
}

void
convert3DMeshToAllTet4(ReplicatedMesh & mesh,
                       const std::vector<std::pair<dof_id_type, bool>> & elems_to_process,
                       std::vector<dof_id_type> & converted_elems_ids_to_cut,
                       std::vector<dof_id_type> & converted_elems_ids_to_retain,
                       const subdomain_id_type block_id_to_remove,
                       const bool delete_block_to_remove)
{
  for (const auto & elem_to_process : elems_to_process)
  {
    switch (mesh.elem_ptr(elem_to_process.first)->type())
    {
      case ElemType::HEX8:
        hexElemSplitter(mesh,
                        elem_to_process.first,
                        elem_to_process.second ? converted_elems_ids_to_cut
                                               : converted_elems_ids_to_retain);
        mesh.elem_ptr(elem_to_process.first)->subdomain_id() = block_id_to_remove;
        break;
      case ElemType::PYRAMID5:
        pyramidElemSplitter(mesh,
                            elem_to_process.first,
                            elem_to_process.second ? converted_elems_ids_to_cut
                                                   : converted_elems_ids_to_retain);
        mesh.elem_ptr(elem_to_process.first)->subdomain_id() = block_id_to_remove;
        break;
      case ElemType::PRISM6:
        prismElemSplitter(mesh,
                          elem_to_process.first,
                          elem_to_process.second ? converted_elems_ids_to_cut
                                                 : converted_elems_ids_to_retain);
        mesh.elem_ptr(elem_to_process.first)->subdomain_id() = block_id_to_remove;
        break;
      case ElemType::TET4:
        if (elem_to_process.second)
          converted_elems_ids_to_cut.push_back(elem_to_process.first);
        else
          converted_elems_ids_to_retain.push_back(elem_to_process.first);
        break;
      default:
        mooseError("Unexpected element type.");
    }
  }

  if (delete_block_to_remove)
  {
    for (auto elem_it = mesh.active_subdomain_elements_begin(block_id_to_remove);
         elem_it != mesh.active_subdomain_elements_end(block_id_to_remove);
         elem_it++)
      mesh.delete_elem(*elem_it);

    mesh.contract();
    mesh.prepare_for_use();
  }
}

void
convert3DMeshToAllTet4(ReplicatedMesh & mesh)
{
  // Subdomain ID for new utility blocks must be new
  std::set<subdomain_id_type> subdomain_ids_set;
  mesh.subdomain_ids(subdomain_ids_set);
  const subdomain_id_type max_subdomain_id = *subdomain_ids_set.rbegin();
  const subdomain_id_type block_id_to_remove = max_subdomain_id + 1;
  std::vector<std::pair<dof_id_type, bool>> original_elems;

  for (auto elem_it = mesh.active_elements_begin(); elem_it != mesh.active_elements_end();
       elem_it++)
  {
    if ((*elem_it)->default_order() != Order::FIRST)
      mooseError("Only first order elements are supported for cutting.");
    original_elems.push_back(std::make_pair((*elem_it)->id(), false));
  }

  std::vector<dof_id_type> converted_elems_ids_to_cut;
  std::vector<dof_id_type> converted_elems_ids_to_retain;

  convert3DMeshToAllTet4(mesh,
                         original_elems,
                         converted_elems_ids_to_cut,
                         converted_elems_ids_to_retain,
                         block_id_to_remove,
                         true);
}
}
