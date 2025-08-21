//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "MooseMeshElementConversionUtils.h"
#include "MooseError.h"
#include "MathUtils.h"
#include "MooseMeshUtils.h"

#include "libmesh/elem.h"
#include "libmesh/enum_order.h"
#include "libmesh/boundary_info.h"
#include "libmesh/mesh_base.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/utility.h"
#include "libmesh/cell_tet4.h"
#include "libmesh/face_tri3.h"
#include "libmesh/cell_pyramid5.h"

using namespace libMesh;

namespace MooseMeshElementConversionUtils
{
void
hexElemSplitter(ReplicatedMesh & mesh,
                const std::vector<libMesh::BoundaryInfo::BCTuple> & bdry_side_list,
                const dof_id_type elem_id,
                std::vector<dof_id_type> & converted_elems_ids)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elem_side_list.resize(6);
  elementBoundaryInfoCollector(bdry_side_list, elem_id, 6, elem_side_list);

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  std::vector<std::vector<unsigned int>> opt_option;
  std::vector<const Node *> elem_node_list = {mesh.elem_ptr(elem_id)->node_ptr(0),
                                              mesh.elem_ptr(elem_id)->node_ptr(1),
                                              mesh.elem_ptr(elem_id)->node_ptr(2),
                                              mesh.elem_ptr(elem_id)->node_ptr(3),
                                              mesh.elem_ptr(elem_id)->node_ptr(4),
                                              mesh.elem_ptr(elem_id)->node_ptr(5),
                                              mesh.elem_ptr(elem_id)->node_ptr(6),
                                              mesh.elem_ptr(elem_id)->node_ptr(7)};
  std::vector<std::vector<unsigned int>> rotated_tet_face_indices;

  std::vector<std::vector<const Node *>> optimized_node_list;
  hexNodesToTetNodesDeterminer(elem_node_list, rotated_tet_face_indices, optimized_node_list);

  std::vector<Elem *> elems_Tet4;
  for (const auto i : index_range(optimized_node_list))
  {
    auto new_elem = std::make_unique<Tet4>();
    new_elem->set_node(0, const_cast<Node *>(optimized_node_list[i][0]));
    new_elem->set_node(1, const_cast<Node *>(optimized_node_list[i][1]));
    new_elem->set_node(2, const_cast<Node *>(optimized_node_list[i][2]));
    new_elem->set_node(3, const_cast<Node *>(optimized_node_list[i][3]));
    new_elem->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id();
    elems_Tet4.push_back(mesh.add_elem(std::move(new_elem)));
    converted_elems_ids.push_back(elems_Tet4.back()->id());

    for (unsigned int j = 0; j < 4; j++)
    {
      // A hex element has 6 faces indexed from 0 to 5
      // a <6 value indicates that the face of the tet element corresponds to the face of the
      // original hex; a =6 value means the face of the tet is an interior face of the hex
      if (rotated_tet_face_indices[i][j] < 6)
      {
        for (const auto & side_info : elem_side_list[rotated_tet_face_indices[i][j]])
          boundary_info.add_side(elems_Tet4.back(), j, side_info);
      }
    }
  }

  // Retain element extra integers
  for (unsigned int i = 0; i < 6; i++)
    for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    {
      elems_Tet4[i]->set_extra_integer(j, exist_extra_ids[j]);
    }
}

void
prismElemSplitter(ReplicatedMesh & mesh,
                  const std::vector<libMesh::BoundaryInfo::BCTuple> & bdry_side_list,
                  const dof_id_type elem_id,
                  std::vector<dof_id_type> & converted_elems_ids)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elementBoundaryInfoCollector(bdry_side_list, elem_id, 5, elem_side_list);

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);

  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  std::vector<const Node *> elem_node_list = {mesh.elem_ptr(elem_id)->node_ptr(0),
                                              mesh.elem_ptr(elem_id)->node_ptr(1),
                                              mesh.elem_ptr(elem_id)->node_ptr(2),
                                              mesh.elem_ptr(elem_id)->node_ptr(3),
                                              mesh.elem_ptr(elem_id)->node_ptr(4),
                                              mesh.elem_ptr(elem_id)->node_ptr(5)};
  std::vector<std::vector<unsigned int>> rotated_tet_face_indices;
  std::vector<std::vector<const Node *>> optimized_node_list;
  prismNodesToTetNodesDeterminer(elem_node_list, rotated_tet_face_indices, optimized_node_list);

  std::vector<Elem *> elems_Tet4;
  for (const auto i : index_range(optimized_node_list))
  {
    auto new_elem = std::make_unique<Tet4>();
    new_elem->set_node(0, const_cast<Node *>(optimized_node_list[i][0]));
    new_elem->set_node(1, const_cast<Node *>(optimized_node_list[i][1]));
    new_elem->set_node(2, const_cast<Node *>(optimized_node_list[i][2]));
    new_elem->set_node(3, const_cast<Node *>(optimized_node_list[i][3]));
    new_elem->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id();
    elems_Tet4.push_back(mesh.add_elem(std::move(new_elem)));
    converted_elems_ids.push_back(elems_Tet4.back()->id());

    for (unsigned int j = 0; j < 4; j++)
    {
      // A prism element has 5 faces indexed from 0 to 4
      // a <4 value indicates that the face of the tet element corresponds to the face of the
      // original prism; a =5 value means the face of the tet is an interior face of the prism
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
                    const std::vector<libMesh::BoundaryInfo::BCTuple> & bdry_side_list,
                    const dof_id_type elem_id,
                    std::vector<dof_id_type> & converted_elems_ids)
{
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  // Create a list of sidesets involving the element to be split
  std::vector<std::vector<boundary_id_type>> elem_side_list;
  elementBoundaryInfoCollector(bdry_side_list, elem_id, 5, elem_side_list);

  const unsigned int n_elem_extra_ids = mesh.n_elem_integers();
  std::vector<dof_id_type> exist_extra_ids(n_elem_extra_ids);
  // Record all the element extra integers of the original quad element
  for (unsigned int j = 0; j < n_elem_extra_ids; j++)
    exist_extra_ids[j] = mesh.elem_ptr(elem_id)->get_extra_integer(j);

  std::vector<const Node *> elem_node_list = {mesh.elem_ptr(elem_id)->node_ptr(0),
                                              mesh.elem_ptr(elem_id)->node_ptr(1),
                                              mesh.elem_ptr(elem_id)->node_ptr(2),
                                              mesh.elem_ptr(elem_id)->node_ptr(3),
                                              mesh.elem_ptr(elem_id)->node_ptr(4)};
  std::vector<std::vector<unsigned int>> rotated_tet_face_indices;
  std::vector<std::vector<const Node *>> optimized_node_list;
  pyramidNodesToTetNodesDeterminer(elem_node_list, rotated_tet_face_indices, optimized_node_list);

  std::vector<Elem *> elems_Tet4;
  for (const auto i : index_range(optimized_node_list))
  {
    auto new_elem = std::make_unique<Tet4>();
    new_elem->set_node(0, const_cast<Node *>(optimized_node_list[i][0]));
    new_elem->set_node(1, const_cast<Node *>(optimized_node_list[i][1]));
    new_elem->set_node(2, const_cast<Node *>(optimized_node_list[i][2]));
    new_elem->set_node(3, const_cast<Node *>(optimized_node_list[i][3]));
    new_elem->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id();
    elems_Tet4.push_back(mesh.add_elem(std::move(new_elem)));
    converted_elems_ids.push_back(elems_Tet4.back()->id());

    for (unsigned int j = 0; j < 4; j++)
    {
      // A pyramid element has 5 faces indexed from 0 to 4
      // a <4 value indicates that the face of the tet element corresponds to the face of the
      // original pyramid; a =5 value means the face of the tet is an interior face of the pyramid
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

void
hexNodesToTetNodesDeterminer(std::vector<const Node *> & hex_nodes,
                             std::vector<std::vector<unsigned int>> & rotated_tet_face_indices,
                             std::vector<std::vector<const Node *>> & tet_nodes_list)
{
  // Find the node with the minimum id
  std::vector<dof_id_type> node_ids(8);
  for (unsigned int i = 0; i < 8; i++)
    node_ids[i] = hex_nodes[i]->id();

  const unsigned int min_node_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));
  // Get the index of the three neighbor nodes of the minimum node
  // The order is consistent with the description in nodeRotationHEX8()
  // Then determine the index of the second minimum node
  const auto neighbor_node_indices = neighborNodeIndicesHEX8(min_node_id_index);

  const auto neighbor_node_ids = {node_ids[neighbor_node_indices[0]],
                                  node_ids[neighbor_node_indices[1]],
                                  node_ids[neighbor_node_indices[2]]};
  const unsigned int sec_min_pos =
      std::distance(std::begin(neighbor_node_ids),
                    std::min_element(std::begin(neighbor_node_ids), std::end(neighbor_node_ids)));

  // Rotate the node and face indices based on the identified minimum and second minimum nodes
  // After the rotation, we guarantee that the minimum node is the first node (Node 0)
  // And the second node (Node 1) has the minium global id among the three neighbor nodes of Node 0
  // This makes the splitting process simpler
  std::vector<unsigned int> face_rotation;
  std::vector<unsigned int> rotated_indices;
  nodeRotationHEX8(min_node_id_index, sec_min_pos, face_rotation, rotated_indices);
  std::vector<const Node *> rotated_hex_nodes;
  for (unsigned int i = 0; i < 8; i++)
    rotated_hex_nodes.push_back(hex_nodes[rotated_indices[i]]);

  // Find the selection of each face's cutting direction
  const auto diagonal_directions = quadFaceDiagonalDirectionsHex(rotated_hex_nodes);

  // Based on the determined splitting directions of all the faces, determine the nodes of each
  // resulting TET4 elements after the splitting.
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

  for (const auto & tet_nodes : tet_nodes_set)
  {
    tet_nodes_list.push_back(std::vector<const Node *>());
    for (const auto & tet_node : tet_nodes)
      tet_nodes_list.back().push_back(rotated_hex_nodes[tet_node]);
  }
}

std::vector<bool>
quadFaceDiagonalDirectionsHex(const std::vector<const Node *> & hex_nodes)
{
  // Bottom/Top; Front/Back; Right/Left
  const std::vector<std::vector<unsigned int>> face_indices = {
      {0, 1, 2, 3}, {4, 5, 6, 7}, {0, 1, 5, 4}, {2, 3, 7, 6}, {1, 2, 6, 5}, {3, 0, 4, 7}};
  std::vector<bool> diagonal_directions;
  for (const auto & face_index : face_indices)
  {
    std::vector<const Node *> quad_nodes = {hex_nodes[face_index[0]],
                                            hex_nodes[face_index[1]],
                                            hex_nodes[face_index[2]],
                                            hex_nodes[face_index[3]]};
    diagonal_directions.push_back(quadFaceDiagonalDirection(quad_nodes));
  }
  return diagonal_directions;
}

bool
quadFaceDiagonalDirection(const std::vector<const Node *> & quad_nodes)
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

void
nodeRotationHEX8(const unsigned int min_id_index,
                 const unsigned int sec_min_pos,
                 std::vector<unsigned int> & face_rotation,
                 std::vector<unsigned int> & node_rotation)
{
  // Assuming the original hex element is a cube, the vectors formed by nodes 0-1, 0-3, and 0-4 are
  // overlapped with the x, y, and z axes, respectively. sec_min_pos = 0 means the second minimum
  // node is in the x direction, sec_min_pos = 1 means the second minimum node is in the y
  // direction, and sec_min_pos = 2 means the second minimum node is in the z direction.
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
      {{3, 0, 2, 5, 4, 1}, {0, 4, 1, 2, 3, 5}, {4, 3, 5, 1, 0, 2}},
      {{1, 5, 2, 0, 4, 3}, {5, 4, 3, 2, 1, 0}, {4, 1, 0, 3, 5, 2}},
      {{5, 1, 4, 3, 2, 0}, {2, 5, 3, 0, 1, 4}, {1, 2, 0, 4, 5, 3}},
      {{3, 5, 4, 0, 2, 1}, {5, 2, 1, 4, 3, 0}, {2, 3, 0, 1, 5, 4}},
      {{5, 3, 2, 1, 4, 0}, {4, 5, 1, 0, 3, 2}, {3, 4, 0, 2, 5, 1}}};

  if (min_id_index > 7 || sec_min_pos > 2)
    mooseError("The input node index is out of range.");
  else
  {
    // index: new face index; value: old face index
    face_rotation = preset_face_indices[min_id_index][sec_min_pos];
    node_rotation = preset_indices[min_id_index][sec_min_pos];
  }
}

void
nodeRotationPRISM6(unsigned int min_id_index,
                   std::vector<unsigned int> & face_rotation,
                   std::vector<unsigned int> & node_rotation)
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
    node_rotation = preset_indices[min_id_index];
  }
}

void
prismNodesToTetNodesDeterminer(std::vector<const Node *> & prism_nodes,
                               std::vector<std::vector<unsigned int>> & rotated_tet_face_indices,
                               std::vector<std::vector<const Node *>> & tet_nodes_list)
{
  // Find the node with the minimum id
  std::vector<dof_id_type> node_ids(6);
  for (unsigned int i = 0; i < 6; i++)
    node_ids[i] = prism_nodes[i]->id();

  const unsigned int min_node_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));

  // Rotate the node and face indices based on the identified minimum node
  // After the rotation, we guarantee that the minimum node is the first node (Node 0)
  // This makes the splitting process simpler
  std::vector<unsigned int> face_rotation;
  std::vector<unsigned int> rotated_indices;
  nodeRotationPRISM6(min_node_id_index, face_rotation, rotated_indices);
  std::vector<const Node *> rotated_prism_nodes;
  for (unsigned int i = 0; i < 6; i++)
    rotated_prism_nodes.push_back(prism_nodes[rotated_indices[i]]);

  std::vector<const Node *> key_quad_nodes = {rotated_prism_nodes[1],
                                              rotated_prism_nodes[2],
                                              rotated_prism_nodes[5],
                                              rotated_prism_nodes[4]};

  // Find the selection of each face's cutting direction
  const bool diagonal_direction = quadFaceDiagonalDirection(key_quad_nodes);

  // Based on the determined splitting directions of all the faces, determine the nodes of each
  // resulting TET4 elements after the splitting.
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

  for (const auto & tet_nodes : tet_nodes_set)
  {
    tet_nodes_list.push_back(std::vector<const Node *>());
    for (const auto & tet_node : tet_nodes)
      tet_nodes_list.back().push_back(rotated_prism_nodes[tet_node]);
  }
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

void
nodeRotationPYRAMID5(unsigned int min_id_index,
                     std::vector<unsigned int> & face_rotation,
                     std::vector<unsigned int> & node_rotation)
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
    node_rotation = preset_indices[min_id_index];
  }
}

void
pyramidNodesToTetNodesDeterminer(std::vector<const Node *> & pyramid_nodes,
                                 std::vector<std::vector<unsigned int>> & rotated_tet_face_indices,
                                 std::vector<std::vector<const Node *>> & tet_nodes_list)
{
  // Find the node with the minimum id, ignoring the top node
  std::vector<dof_id_type> node_ids(4);
  for (unsigned int i = 0; i < 4; i++)
    node_ids[i] = pyramid_nodes[i]->id();

  const unsigned int min_node_id_index = std::distance(
      std::begin(node_ids), std::min_element(std::begin(node_ids), std::end(node_ids)));

  // Rotate the node and face indices based on the identified minimum nodes
  // After the rotation, we guarantee that the minimum node is the first node (Node 0)
  // This makes the splitting process simpler
  std::vector<unsigned int> face_rotation;
  std::vector<unsigned int> rotated_indices;
  nodeRotationPYRAMID5(min_node_id_index, face_rotation, rotated_indices);
  std::vector<const Node *> rotated_pyramid_nodes;
  for (unsigned int i = 0; i < 5; i++)
    rotated_pyramid_nodes.push_back(pyramid_nodes[rotated_indices[i]]);

  // There is only one quad face in a pyramid element, so the splitting selection is binary
  const std::vector<std::vector<unsigned int>> tet_nodes_set = {{0, 1, 2, 4}, {0, 2, 3, 4}};
  const std::vector<std::vector<unsigned int>> tet_face_indices = {{4, 0, 1, 5}, {4, 5, 2, 3}};

  // Based on the determined splitting direction, determine the nodes of each resulting TET4
  // elements after the splitting.
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

  for (const auto & tet_nodes : tet_nodes_set)
  {
    tet_nodes_list.push_back(std::vector<const Node *>());
    for (const auto & tet_node : tet_nodes)
      tet_nodes_list.back().push_back(rotated_pyramid_nodes[tet_node]);
  }
}

void
convert3DMeshToAllTet4(ReplicatedMesh & mesh,
                       const std::vector<std::pair<dof_id_type, bool>> & elems_to_process,
                       std::vector<dof_id_type> & converted_elems_ids_to_track,
                       const subdomain_id_type block_id_to_remove,
                       const bool delete_block_to_remove)
{
  std::vector<dof_id_type> converted_elems_ids_to_retain;
  // Build boundary information of the mesh
  BoundaryInfo & boundary_info = mesh.get_boundary_info();
  const auto bdry_side_list = boundary_info.build_side_list();
  for (const auto & elem_to_process : elems_to_process)
  {
    switch (mesh.elem_ptr(elem_to_process.first)->type())
    {
      case ElemType::HEX8:
        hexElemSplitter(mesh,
                        bdry_side_list,
                        elem_to_process.first,
                        elem_to_process.second ? converted_elems_ids_to_track
                                               : converted_elems_ids_to_retain);
        mesh.elem_ptr(elem_to_process.first)->subdomain_id() = block_id_to_remove;
        break;
      case ElemType::PYRAMID5:
        pyramidElemSplitter(mesh,
                            bdry_side_list,
                            elem_to_process.first,
                            elem_to_process.second ? converted_elems_ids_to_track
                                                   : converted_elems_ids_to_retain);
        mesh.elem_ptr(elem_to_process.first)->subdomain_id() = block_id_to_remove;
        break;
      case ElemType::PRISM6:
        prismElemSplitter(mesh,
                          bdry_side_list,
                          elem_to_process.first,
                          elem_to_process.second ? converted_elems_ids_to_track
                                                 : converted_elems_ids_to_retain);
        mesh.elem_ptr(elem_to_process.first)->subdomain_id() = block_id_to_remove;
        break;
      case ElemType::TET4:
        if (elem_to_process.second)
          converted_elems_ids_to_track.push_back(elem_to_process.first);
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

  std::vector<dof_id_type> converted_elems_ids_to_track;

  convert3DMeshToAllTet4(
      mesh, original_elems, converted_elems_ids_to_track, block_id_to_remove, true);
}

void
elementBoundaryInfoCollector(const std::vector<libMesh::BoundaryInfo::BCTuple> & bdry_side_list,
                             const dof_id_type elem_id,
                             const unsigned short n_elem_sides,
                             std::vector<std::vector<boundary_id_type>> & elem_side_list)
{
  elem_side_list.resize(n_elem_sides);
  const auto selected_bdry_side_list =
      std::equal_range(bdry_side_list.begin(), bdry_side_list.end(), elem_id, BCTupleKeyComp{});
  for (auto selected_bdry_side = selected_bdry_side_list.first;
       selected_bdry_side != selected_bdry_side_list.second;
       ++selected_bdry_side)
  {
    elem_side_list[std::get<1>(*selected_bdry_side)].push_back(std::get<2>(*selected_bdry_side));
  }
}

void
convertElem(ReplicatedMesh & mesh,
            const dof_id_type & elem_id,
            const std::vector<unsigned int> & side_indices,
            const std::vector<std::vector<boundary_id_type>> & elem_side_info,
            const SubdomainID & subdomain_id_shift_base)
{
  const auto & elem_type = mesh.elem_ptr(elem_id)->type();
  switch (elem_type)
  {
    case HEX8:
      // HEX8 to PYRAMID5 (+2*subdomain_id_shift_base)
      // HEX8 to TET4 (+subdomain_id_shift_base)
      convertHex8Elem(mesh, elem_id, side_indices, elem_side_info, subdomain_id_shift_base);
      break;
    case PRISM6:
      // PRISM6 to TET4 (+subdomain_id_shift_base)
      // PRISM6 to PYRAMID5 (+2*subdomain_id_shift_base)
      convertPrism6Elem(mesh, elem_id, side_indices, elem_side_info, subdomain_id_shift_base);
      break;
    case PYRAMID5:
      // PYRAMID5 to TET4 (+subdomain_id_shift_base)
      convertPyramid5Elem(mesh, elem_id, elem_side_info, subdomain_id_shift_base);
      break;
    default:
      mooseAssert(false,
                  "The provided element type '" + std::to_string(elem_type) +
                      "' is not supported and is not supposed to be passed to this function. "
                      "Only HEX8, PRISM6 and PYRAMID5 are supported.");
  }
}

void
convertHex8Elem(ReplicatedMesh & mesh,
                const dof_id_type & elem_id,
                const std::vector<unsigned int> & side_indices,
                const std::vector<std::vector<boundary_id_type>> & elem_side_info,
                const SubdomainID & subdomain_id_shift_base)
{
  // We add a node at the centroid of the HEX8 element
  // With this node, the HEX8 can be converted into 6 PYRAMID5 elements
  // For the PYRAMID5 element at the 'side_indices', they can further be converted into 2 TET4
  // elements
  const Point elem_cent = mesh.elem_ptr(elem_id)->true_centroid();
  auto new_node = mesh.add_point(elem_cent);
  for (const auto & i_side : make_range(mesh.elem_ptr(elem_id)->n_sides()))
  {
    if (std::find(side_indices.begin(), side_indices.end(), i_side) != side_indices.end())
      createUnitTet4FromHex8(
          mesh, elem_id, i_side, new_node, elem_side_info[i_side], subdomain_id_shift_base);
    else
      createUnitPyramid5FromHex8(
          mesh, elem_id, i_side, new_node, elem_side_info[i_side], subdomain_id_shift_base);
  }
}

void
createUnitPyramid5FromHex8(ReplicatedMesh & mesh,
                           const dof_id_type & elem_id,
                           const unsigned int & side_index,
                           const Node * new_node,
                           const std::vector<boundary_id_type> & side_info,
                           const SubdomainID & subdomain_id_shift_base)
{
  auto new_elem = std::make_unique<Pyramid5>();
  new_elem->set_node(0, mesh.elem_ptr(elem_id)->side_ptr(side_index)->node_ptr(3));
  new_elem->set_node(1, mesh.elem_ptr(elem_id)->side_ptr(side_index)->node_ptr(2));
  new_elem->set_node(2, mesh.elem_ptr(elem_id)->side_ptr(side_index)->node_ptr(1));
  new_elem->set_node(3, mesh.elem_ptr(elem_id)->side_ptr(side_index)->node_ptr(0));
  new_elem->set_node(4, const_cast<Node *>(new_node));
  new_elem->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + subdomain_id_shift_base * 2;
  auto new_elem_ptr = mesh.add_elem(std::move(new_elem));
  retainEEID(mesh, elem_id, new_elem_ptr);
  for (const auto & bid : side_info)
    mesh.get_boundary_info().add_side(new_elem_ptr, 4, bid);
}

void
createUnitTet4FromHex8(ReplicatedMesh & mesh,
                       const dof_id_type & elem_id,
                       const unsigned int & side_index,
                       const Node * new_node,
                       const std::vector<boundary_id_type> & side_info,
                       const SubdomainID & subdomain_id_shift_base)
{
  // We want to make sure that the QUAD4 is divided by the diagonal that involves the node with
  // the lowest node id This may help maintain consistency for future applications
  unsigned int lid_index = 0;
  for (const auto & i : make_range(1, 4))
  {
    if (mesh.elem_ptr(elem_id)->side_ptr(side_index)->node_ptr(i)->id() <
        mesh.elem_ptr(elem_id)->side_ptr(side_index)->node_ptr(lid_index)->id())
      lid_index = i;
  }

  auto new_elem_0 = std::make_unique<Tet4>();
  new_elem_0->set_node(0,
                       mesh.elem_ptr(elem_id)
                           ->side_ptr(side_index)
                           ->node_ptr(MathUtils::euclideanMod(2 - lid_index % 2, 4)));
  new_elem_0->set_node(1,
                       mesh.elem_ptr(elem_id)
                           ->side_ptr(side_index)
                           ->node_ptr(MathUtils::euclideanMod(1 - lid_index % 2, 4)));
  new_elem_0->set_node(2,
                       mesh.elem_ptr(elem_id)
                           ->side_ptr(side_index)
                           ->node_ptr(MathUtils::euclideanMod(0 - lid_index % 2, 4)));
  new_elem_0->set_node(3, const_cast<Node *>(new_node));
  new_elem_0->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + subdomain_id_shift_base;
  auto new_elem_ptr_0 = mesh.add_elem(std::move(new_elem_0));
  retainEEID(mesh, elem_id, new_elem_ptr_0);

  auto new_elem_1 = std::make_unique<Tet4>();
  new_elem_1->set_node(0,
                       mesh.elem_ptr(elem_id)
                           ->side_ptr(side_index)
                           ->node_ptr(MathUtils::euclideanMod(3 - lid_index % 2, 4)));
  new_elem_1->set_node(1,
                       mesh.elem_ptr(elem_id)
                           ->side_ptr(side_index)
                           ->node_ptr(MathUtils::euclideanMod(2 - lid_index % 2, 4)));
  new_elem_1->set_node(2,
                       mesh.elem_ptr(elem_id)
                           ->side_ptr(side_index)
                           ->node_ptr(MathUtils::euclideanMod(0 - lid_index % 2, 4)));
  new_elem_1->set_node(3, const_cast<Node *>(new_node));
  new_elem_1->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + subdomain_id_shift_base;
  auto new_elem_ptr_1 = mesh.add_elem(std::move(new_elem_1));
  retainEEID(mesh, elem_id, new_elem_ptr_1);

  for (const auto & bid : side_info)
  {
    mesh.get_boundary_info().add_side(new_elem_ptr_0, 0, bid);
    mesh.get_boundary_info().add_side(new_elem_ptr_1, 0, bid);
  }
}

void
convertPrism6Elem(ReplicatedMesh & mesh,
                  const dof_id_type & elem_id,
                  const std::vector<unsigned int> & side_indices,
                  const std::vector<std::vector<boundary_id_type>> & elem_side_info,
                  const SubdomainID & subdomain_id_shift_base)
{
  // We add a node at the centroid of the PRISM6 element
  // With this node, the PRISM6 can be converted into 3 PYRAMID5 elements and 2 TET4 elements
  // For the PYRAMID5 element, it can further be converted into 2 TET3 elements
  const Point elem_cent = mesh.elem_ptr(elem_id)->true_centroid();
  auto new_node = mesh.add_point(elem_cent);
  for (const auto & i_side : make_range(mesh.elem_ptr(elem_id)->n_sides()))
  {
    if (i_side % 4 == 0 ||
        std::find(side_indices.begin(), side_indices.end(), i_side) != side_indices.end())
      createUnitTet4FromPrism6(
          mesh, elem_id, i_side, new_node, elem_side_info[i_side], subdomain_id_shift_base);
    else
      createUnitPyramid5FromPrism6(
          mesh, elem_id, i_side, new_node, elem_side_info[i_side], subdomain_id_shift_base);
  }
}

void
createUnitTet4FromPrism6(ReplicatedMesh & mesh,
                         const dof_id_type & elem_id,
                         const unsigned int & side_index,
                         const Node * new_node,
                         const std::vector<boundary_id_type> & side_info,
                         const SubdomainID & subdomain_id_shift_base)
{
  // For side 1 and side 4, they are already TRI3, so only one TET is created
  // For side 0, 2, and 3, they are QUAD4, so we create 2 TETs
  // We want to make sure that the QUAD4 is divided by the diagonal that involves
  // the node with the lowest node id This may help maintain consistency for future applications
  bool is_side_quad = (side_index % 4 != 0);
  unsigned int lid_index = 0;
  if (is_side_quad)
    for (const auto & i : make_range(1, 4))
    {
      if (mesh.elem_ptr(elem_id)->side_ptr(side_index)->node_ptr(i)->id() <
          mesh.elem_ptr(elem_id)->side_ptr(side_index)->node_ptr(lid_index)->id())
        lid_index = i;
    }
  // For a TRI3 side, lid_index is always 0, so the indices are always 2,1,0 here
  auto new_elem_0 = std::make_unique<Tet4>();
  new_elem_0->set_node(0,
                       mesh.elem_ptr(elem_id)
                           ->side_ptr(side_index)
                           ->node_ptr(MathUtils::euclideanMod(2 - lid_index % 2, 4)));
  new_elem_0->set_node(1,
                       mesh.elem_ptr(elem_id)
                           ->side_ptr(side_index)
                           ->node_ptr(MathUtils::euclideanMod(1 - lid_index % 2, 4)));
  new_elem_0->set_node(2,
                       mesh.elem_ptr(elem_id)
                           ->side_ptr(side_index)
                           ->node_ptr(MathUtils::euclideanMod(0 - lid_index % 2, 4)));
  new_elem_0->set_node(3, const_cast<Node *>(new_node));
  new_elem_0->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + subdomain_id_shift_base;
  auto new_elem_ptr_0 = mesh.add_elem(std::move(new_elem_0));
  retainEEID(mesh, elem_id, new_elem_ptr_0);

  Elem * new_elem_ptr_1 = nullptr;
  if (is_side_quad)
  {
    auto new_elem_1 = std::make_unique<Tet4>();
    new_elem_1->set_node(0,
                         mesh.elem_ptr(elem_id)
                             ->side_ptr(side_index)
                             ->node_ptr(MathUtils::euclideanMod(3 - lid_index % 2, 4)));
    new_elem_1->set_node(1,
                         mesh.elem_ptr(elem_id)
                             ->side_ptr(side_index)
                             ->node_ptr(MathUtils::euclideanMod(2 - lid_index % 2, 4)));
    new_elem_1->set_node(2,
                         mesh.elem_ptr(elem_id)
                             ->side_ptr(side_index)
                             ->node_ptr(MathUtils::euclideanMod(0 - lid_index % 2, 4)));
    new_elem_1->set_node(3, const_cast<Node *>(new_node));
    new_elem_1->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + subdomain_id_shift_base;
    new_elem_ptr_1 = mesh.add_elem(std::move(new_elem_1));
    retainEEID(mesh, elem_id, new_elem_ptr_1);
  }

  for (const auto & bid : side_info)
  {
    mesh.get_boundary_info().add_side(new_elem_ptr_0, 0, bid);
    if (new_elem_ptr_1)
      mesh.get_boundary_info().add_side(new_elem_ptr_1, 0, bid);
  }
}

void
createUnitPyramid5FromPrism6(ReplicatedMesh & mesh,
                             const dof_id_type & elem_id,
                             const unsigned int & side_index,
                             const Node * new_node,
                             const std::vector<boundary_id_type> & side_info,
                             const SubdomainID & subdomain_id_shift_base)
{
  // Same as Hex8
  createUnitPyramid5FromHex8(
      mesh, elem_id, side_index, new_node, side_info, subdomain_id_shift_base);
}

void
convertPyramid5Elem(ReplicatedMesh & mesh,
                    const dof_id_type & elem_id,
                    const std::vector<std::vector<boundary_id_type>> & elem_side_info,
                    const SubdomainID & subdomain_id_shift_base)
{
  // A Pyramid5 element has only one QUAD4 face, so we can convert it to 2 TET4 elements
  unsigned int lid_index = 0;
  for (const auto & i : make_range(1, 4))
  {
    if (mesh.elem_ptr(elem_id)->side_ptr(4)->node_ptr(i)->id() <
        mesh.elem_ptr(elem_id)->side_ptr(4)->node_ptr(lid_index)->id())
      lid_index = i;
  }
  auto new_elem_0 = std::make_unique<Tet4>();
  new_elem_0->set_node(
      0,
      mesh.elem_ptr(elem_id)->side_ptr(4)->node_ptr(MathUtils::euclideanMod(2 - lid_index % 2, 4)));
  new_elem_0->set_node(
      1,
      mesh.elem_ptr(elem_id)->side_ptr(4)->node_ptr(MathUtils::euclideanMod(1 - lid_index % 2, 4)));
  new_elem_0->set_node(
      2,
      mesh.elem_ptr(elem_id)->side_ptr(4)->node_ptr(MathUtils::euclideanMod(0 - lid_index % 2, 4)));
  new_elem_0->set_node(3, mesh.elem_ptr(elem_id)->node_ptr(4));
  new_elem_0->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + subdomain_id_shift_base;
  auto new_elem_ptr_0 = mesh.add_elem(std::move(new_elem_0));
  retainEEID(mesh, elem_id, new_elem_ptr_0);

  auto new_elem_1 = std::make_unique<Tet4>();
  new_elem_1->set_node(
      0,
      mesh.elem_ptr(elem_id)->side_ptr(4)->node_ptr(MathUtils::euclideanMod(3 - lid_index % 2, 4)));
  new_elem_1->set_node(
      1,
      mesh.elem_ptr(elem_id)->side_ptr(4)->node_ptr(MathUtils::euclideanMod(2 - lid_index % 2, 4)));
  new_elem_1->set_node(
      2,
      mesh.elem_ptr(elem_id)->side_ptr(4)->node_ptr(MathUtils::euclideanMod(0 - lid_index % 2, 4)));
  new_elem_1->set_node(3, mesh.elem_ptr(elem_id)->node_ptr(4));
  new_elem_1->subdomain_id() = mesh.elem_ptr(elem_id)->subdomain_id() + subdomain_id_shift_base;
  auto new_elem_ptr_1 = mesh.add_elem(std::move(new_elem_1));
  retainEEID(mesh, elem_id, new_elem_ptr_1);

  for (const auto & bid : elem_side_info[0])
    mesh.get_boundary_info().add_side(new_elem_ptr_0, 2 - lid_index % 2, bid);
  for (const auto & bid : elem_side_info[1])
    if (lid_index % 2)
      mesh.get_boundary_info().add_side(new_elem_ptr_1, 2, bid);
    else
      mesh.get_boundary_info().add_side(new_elem_ptr_0, 1, bid);
  for (const auto & bid : elem_side_info[2])
    mesh.get_boundary_info().add_side(new_elem_ptr_1, 2 + lid_index % 2, bid);
  for (const auto & bid : elem_side_info[3])
    if (lid_index % 2)
      mesh.get_boundary_info().add_side(new_elem_ptr_0, 1, bid);
    else
      mesh.get_boundary_info().add_side(new_elem_ptr_1, 3, bid);
  for (const auto & bid : elem_side_info[4])
  {
    mesh.get_boundary_info().add_side(new_elem_ptr_0, 0, bid);
    mesh.get_boundary_info().add_side(new_elem_ptr_1, 0, bid);
  }
}

void
retainEEID(ReplicatedMesh & mesh, const dof_id_type & elem_id, Elem * new_elem_ptr)
{
  const unsigned int n_eeid = mesh.n_elem_integers();
  for (const auto & i : make_range(n_eeid))
    new_elem_ptr->set_extra_integer(i, mesh.elem_ptr(elem_id)->get_extra_integer(i));
}

void
transitionLayerGenerator(ReplicatedMesh & mesh,
                         const std::vector<BoundaryName> & boundary_names,
                         const unsigned int conversion_element_layer_number,
                         const bool external_boundaries_checking)
{
  // The base subdomain ID to shift the original elements because of the element type change
  const auto sid_shift_base = MooseMeshUtils::getNextFreeSubdomainID(mesh);

  // It would be convenient to have a single boundary id instead of a vector.
  const auto uniform_tmp_bid = MooseMeshUtils::getNextFreeBoundaryID(mesh);

  // Check the boundaries and merge them
  std::vector<boundary_id_type> boundary_ids;
  for (const auto & sideset : boundary_names)
  {
    if (!MooseMeshUtils::hasBoundaryName(mesh, sideset))
      throw MooseException("The provided boundary '", sideset, "' was not found within the mesh");
    boundary_ids.push_back(MooseMeshUtils::getBoundaryID(sideset, mesh));
    MooseMeshUtils::changeBoundaryId(mesh, boundary_ids.back(), uniform_tmp_bid, false);
  }

  auto & sideset_map = mesh.get_boundary_info().get_sideset_map();
  auto side_list = mesh.get_boundary_info().build_side_list();

  std::vector<std::pair<dof_id_type, std::vector<unsigned int>>> elems_list;
  std::vector<std::set<dof_id_type>> layered_elems_list;
  layered_elems_list.push_back(std::set<dof_id_type>());
  // Need to collect the list of elements that need to be converted
  if (external_boundaries_checking && !mesh.is_prepared())
    mesh.find_neighbors();
  for (const auto & side_info : side_list)
  {
    if (std::get<2>(side_info) == uniform_tmp_bid)
    {
      // Check if the involved side is TRI3 or QUAD4
      // We do not limit the element type in the input mesh
      // As long as the involved boundaries only consist of TRI3 and QUAD4 sides,
      // this generator will work
      // As the side element of a quadratic element is still a linear element in libMesh,
      // we need to check the element's default_side_order() first
      if (mesh.elem_ptr(std::get<0>(side_info))->default_side_order() != 1)
        throw MooseException(
            "The provided boundary set contains non-linear side elements, which is not supported.");
      const auto side_type =
          mesh.elem_ptr(std::get<0>(side_info))->side_ptr(std::get<1>(side_info))->type();
      layered_elems_list.back().emplace(std::get<0>(side_info));
      if (conversion_element_layer_number == 1)
      {
        if (side_type == TRI3)
          continue; // Already TRI3, no need to convert
        else if (side_type == QUAD4)
        {
          if (elems_list.size() && elems_list.back().first == std::get<0>(side_info))
            elems_list.back().second.push_back(std::get<1>(side_info));
          else
            elems_list.push_back(std::make_pair(
                std::get<0>(side_info), std::vector<unsigned int>({std::get<1>(side_info)})));
        }
        else if (side_type == C0POLYGON)
          throw MooseException("The provided boundary set contains C0POLYGON side elements, which "
                               "is not supported.");
        else
          mooseAssert(false,
                      "Impossible scenario: a linear side element that is neither TRI3 nor QUAD4.");
      }
      if (external_boundaries_checking)
        if (mesh.elem_ptr(std::get<0>(side_info))->neighbor_ptr(std::get<1>(side_info)))
          throw MooseException(
              "The provided boundary contains non-external sides, which is required when "
              "external_boundaries_checking is enabled.");
    }
  }

  if (conversion_element_layer_number > 1)
  {
    std::set<dof_id_type> total_elems_set(layered_elems_list.back());

    while (layered_elems_list.back().size() &&
           layered_elems_list.size() < conversion_element_layer_number)
    {
      layered_elems_list.push_back(std::set<dof_id_type>());
      for (const auto & elem_id : *(layered_elems_list.end() - 2))
      {
        for (const auto & i_side : make_range(mesh.elem_ptr(elem_id)->n_sides()))
        {
          if (mesh.elem_ptr(elem_id)->neighbor_ptr(i_side) != nullptr)
          {
            const auto & neighbor_id = mesh.elem_ptr(elem_id)->neighbor_ptr(i_side)->id();
            if (total_elems_set.find(neighbor_id) == total_elems_set.end())
            {
              layered_elems_list.back().emplace(neighbor_id);
              total_elems_set.emplace(neighbor_id);
            }
          }
        }
      }
    }
  }

  // Remove the last empty layer
  if (layered_elems_list.back().empty())
    layered_elems_list.pop_back();

  if (conversion_element_layer_number > layered_elems_list.size())
    throw MooseException("There is fewer layers of elements in the input mesh than the requested "
                         "number of layers to convert.");

  std::vector<std::pair<dof_id_type, bool>> original_elems;
  // construct a list of the element to convert to tet4
  // Convert at most n_layer_conversion layers of elements
  const unsigned int n_layer_conversion = layered_elems_list.size() - 1;
  for (unsigned int i = 0; i < n_layer_conversion; ++i)
    for (const auto & elem_id : layered_elems_list[i])
    {
      // As these elements will become TET4 elements, we need to shift the subdomain ID
      // But we do not need to convert original TET4 elements
      if (mesh.elem_ptr(elem_id)->type() != TET4)
      {
        original_elems.push_back(std::make_pair(elem_id, false));
        mesh.elem_ptr(elem_id)->subdomain_id() += sid_shift_base;
      }
    }

  const subdomain_id_type block_id_to_remove = sid_shift_base * 3;

  std::vector<dof_id_type> converted_elems_ids_to_track;
  MooseMeshElementConversionUtils::convert3DMeshToAllTet4(
      mesh, original_elems, converted_elems_ids_to_track, block_id_to_remove, false);

  // Now we need to convert the elements on the transition layer
  // First, we need to identify that the sides that are on the interface with previous layer (all
  // TET layers)
  if (n_layer_conversion)
  {
    for (const auto & elem_id : layered_elems_list[n_layer_conversion])
    {
      for (const auto & i_side : make_range(mesh.elem_ptr(elem_id)->n_sides()))
      {
        if (mesh.elem_ptr(elem_id)->neighbor_ptr(i_side) != nullptr &&
            layered_elems_list[n_layer_conversion - 1].count(
                mesh.elem_ptr(elem_id)->neighbor_ptr(i_side)->id()))
        {
          if (elems_list.size() && elems_list.back().first == elem_id)
            elems_list.back().second.push_back(i_side);
          else
            elems_list.push_back(std::make_pair(elem_id, std::vector<unsigned int>({i_side})));
        }
      }
    }
  }

  // Now convert the elements
  for (const auto & elem_info : elems_list)
  {
    // Find the involved sidesets of the element so that we can retain them
    std::vector<std::vector<boundary_id_type>> elem_side_info(
        mesh.elem_ptr(elem_info.first)->n_sides());
    auto side_range = sideset_map.equal_range(mesh.elem_ptr(elem_info.first));
    for (auto i = side_range.first; i != side_range.second; ++i)
      elem_side_info[i->second.first].push_back(i->second.second);

    MooseMeshElementConversionUtils::convertElem(
        mesh, elem_info.first, elem_info.second, elem_side_info, sid_shift_base);
  }

  // delete the original elements that were converted
  for (const auto & elem_info : elems_list)
    mesh.elem_ptr(elem_info.first)->subdomain_id() = block_id_to_remove;
  for (auto elem_it = mesh.active_subdomain_elements_begin(block_id_to_remove);
       elem_it != mesh.active_subdomain_elements_end(block_id_to_remove);
       elem_it++)
    mesh.delete_elem(*elem_it);
  // delete temporary boundary id
  mesh.get_boundary_info().remove_id(uniform_tmp_bid);

  mesh.contract();
  mesh.set_isnt_prepared();
}

void
assignConvertedElementsSubdomainNameSuffix(
    ReplicatedMesh & mesh,
    const std::set<subdomain_id_type> & original_subdomain_ids,
    const subdomain_id_type sid_shift_base,
    const SubdomainName & tet_suffix,
    const SubdomainName & pyramid_suffix)
{
  for (const auto & subdomain_id : original_subdomain_ids)
  {
    if (MooseMeshUtils::hasSubdomainID(mesh, subdomain_id + sid_shift_base))
    {
      const SubdomainName new_name =
          (mesh.subdomain_name(subdomain_id).empty() ? std::to_string(subdomain_id)
                                                     : mesh.subdomain_name(subdomain_id)) +
          '_' + tet_suffix;
      if (MooseMeshUtils::hasSubdomainName(mesh, new_name))
        throw MooseException(
            "This suffix for converted TET4 elements results in a subdomain name, " + new_name +
            ", that already exists in the mesh. Please choose a different suffix.");
      mesh.subdomain_name(subdomain_id + sid_shift_base) = new_name;
    }
    if (MooseMeshUtils::hasSubdomainID(mesh, subdomain_id + 2 * sid_shift_base))
    {
      const SubdomainName new_name =
          (mesh.subdomain_name(subdomain_id).empty() ? std::to_string(subdomain_id)
                                                     : mesh.subdomain_name(subdomain_id)) +
          '_' + pyramid_suffix;
      if (MooseMeshUtils::hasSubdomainName(mesh, new_name))
        throw MooseException(
            "This suffix for converted PYRAMID5 elements results in a subdomain name, " + new_name +
            ", that already exists in the mesh. Please choose a different suffix.");
      mesh.subdomain_name(subdomain_id + 2 * sid_shift_base) = new_name;
    }
  }
}
}
