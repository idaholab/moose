//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/replicated_mesh.h"

#include "MooseTypes.h"

namespace MooseMeshElementConversionUtils
{
// Define for comparing the key value of BCTuple
struct BCTupleKeyComp
{
  bool operator()(const libMesh::BoundaryInfo::BCTuple & s, dof_id_type i) const
  {
    return std::get<0>(s) < i;
  }
  bool operator()(dof_id_type i, const libMesh::BoundaryInfo::BCTuple & s) const
  {
    return i < std::get<0>(s);
  }
};

/**
 * Split a HEX8 element into six TET4 elements.
 * @param mesh The mesh to be modified
 * @param bdry_side_list A list that contains the boundary information of the original mesh
 * @param elem_id The id of the element to be split
 * @param converted_elems_ids a vector to record the ids of the newly created TET4 elements
 */
void hexElemSplitter(ReplicatedMesh & mesh,
                     const std::vector<libMesh::BoundaryInfo::BCTuple> & bdry_side_list,
                     const dof_id_type elem_id,
                     std::vector<dof_id_type> & converted_elems_ids);

/**
 * Split a PYRAMID5 element into two TET4 elements.
 * @param mesh The mesh to be modified
 * @param bdry_side_list A list that contains the boundary information of the original mesh
 * @param elem_id The id of the element to be split
 * @param converted_elems_ids a vector to record the ids of the newly created TET4 elements
 */
void pyramidElemSplitter(ReplicatedMesh & mesh,
                         const std::vector<libMesh::BoundaryInfo::BCTuple> & bdry_side_list,
                         const dof_id_type elem_id,
                         std::vector<dof_id_type> & converted_elems_ids);

/**
 * Split a PRISM6 element into three TET4 elements.
 * @param mesh The mesh to be modified
 * @param bdry_side_list A list that contains the boundary information of the original mesh
 * @param elem_id The id of the element to be split
 * @param converted_elems_ids a vector to record the ids of the newly created TET4 elements
 */
void prismElemSplitter(ReplicatedMesh & mesh,
                       const std::vector<libMesh::BoundaryInfo::BCTuple> & bdry_side_list,
                       const dof_id_type elem_id,
                       std::vector<dof_id_type> & converted_elems_ids);

/**
 * Rotate a HEX8 element's nodes to ensure that the node with the minimum id is the first node;
 * and the node among its three neighboring nodes with the minimum id is the second node.
 * @param min_id_index The index of the node with the minimum id
 * @param sec_min_pos The index of the node among its three neighboring nodes with the minimum
 * id (see comments in the function for more details about how the index is defined)
 * @param face_rotation A vector to record the rotation of the faces of the HEX8 element
 * @param node_rotation a vector of node indices that can form a HEX8 element
 */
void nodeRotationHEX8(const unsigned int min_id_index,
                      const unsigned int sec_min_pos,
                      std::vector<unsigned int> & face_rotation,
                      std::vector<unsigned int> & node_rotation);

/**
 * Calculate the indices (within the element nodes) of the three neighboring nodes of a node in a
 * HEX8 element.
 * @param min_id_index The index of the node with the minimum id
 * @return a vector of the three neighboring nodes
 */
std::vector<unsigned int> neighborNodeIndicesHEX8(unsigned int min_id_index);

/**
 * For a vector of rotated nodes that can form a HEX8 element, create a vector of four-node sets
 * that can form TET4 elements to replace the original HEX8 element. All the QUAD4 faces of the
 * HEX8 element will be split by the diagonal line that involves the node with the minimum id of
 * that face.
 * @param hex_nodes A vector of pointers to the nodes that can form a HEX8 element
 * @param rotated_tet_face_indices A vector of vectors of the original face indices of the HEX8
 * element corresponding to the faces of the newly created TET4 elements
 * @param tet_nodes_list a vector of vectors of pointers to the nodes that can form TET4 elements
 */
void hexNodesToTetNodesDeterminer(std::vector<const Node *> & hex_nodes,
                                  std::vector<std::vector<unsigned int>> & rotated_tet_face_indices,
                                  std::vector<std::vector<const Node *>> & tet_nodes_list);

/**
 * For a HEX8 element, determine the direction of the diagonal line of each face that involves the
 * node with the minimum id of that face.
 * @param hex_nodes A vector of pointers to the nodes that can form a HEX8 element
 * @return a vector of boolean values indicating the direction of the diagonal line of each face
 */
std::vector<bool> quadFaceDiagonalDirectionsHex(const std::vector<const Node *> & hex_nodes);

/**
 * For a QUAD4 element, determine the direction of the diagonal line that involves the node with
 * the minimum id of that element.
 * @param quad_nodes A vector of pointers to the nodes that can form a QUAD4 element
 * @return a boolean value indicating the direction of the diagonal line
 */
bool quadFaceDiagonalDirection(const std::vector<const Node *> & quad_nodes);

/**
 * Creates sets of four nodes indices that can form TET4 elements to replace the original HEX8
 * element.
 * @param diagonal_directions A vector of boolean values indicating the direction of the diagonal
 * line of each face; true means the diagonal line is connecting node 0 and node 2, false means the
 * diagonal line is connecting node 1 and node 3 of that quad face
 * @param tet_face_indices A vector of vectors of the original face indices of the HEX8 element
 * corresponding to the faces of the newly created TET4 elements
 * @return a vector of vectors of node indices that can form TET4 elements
 */
std::vector<std::vector<unsigned int>>
tetNodesForHex(const std::vector<bool> diagonal_directions,
               std::vector<std::vector<unsigned int>> & tet_face_indices);

/**
 * Rotate a PRISM6 element nodes to ensure that the node with the minimum id is the first node.
 * @param min_id_index The index of the node, within the prism nodes, with the minimum id
 * @param face_rotation A vector to record the rotation of the faces of the PRISM6 element
 * @param node_rotation a vector of node indices that can form a PRISM6 element
 */
void nodeRotationPRISM6(unsigned int min_id_index,
                        std::vector<unsigned int> & face_rotation,
                        std::vector<unsigned int> & node_rotation);

/**
 * For a rotated nodes that can form a PRISM6 element, create a series of four-node set that can
 * form TET4 elements to replace the original PRISM6 element. All the QUAD4 face of the PRISM6
 * element will be split by the diagonal line that involves the node with the minimum id of that
 * face.
 * @param prism_nodes A vector of pointers to the nodes that can form a PRISM6 element
 * @param rotated_tet_face_indices A vector of vectors of the original face indices of the PRISM6
 * element corresponding to the faces of the newly created TET4 elements
 * @param tet_nodes_list a vector of vectors of pointers to the nodes that can form TET4 elements
 */
void
prismNodesToTetNodesDeterminer(std::vector<const Node *> & prism_nodes,
                               std::vector<std::vector<unsigned int>> & rotated_tet_face_indices,
                               std::vector<std::vector<const Node *>> & tet_nodes_list);

/**
 * Creates sets of four nodes indices that can form TET4 elements to replace the original PRISM6
 * element.
 * @param diagonal_direction A boolean value indicating the direction of the diagonal line of Face
 * 2
 * @param tet_face_indices A vector of vectors of the original face indices of the PRISM6 element
 * corresponding to the faces of the newly created TET4 elements
 * @return a vector of vectors of node indices that can form TET4 elements
 */
std::vector<std::vector<unsigned int>>
tetNodesForPrism(const bool diagonal_direction,
                 std::vector<std::vector<unsigned int>> & tet_face_indices);

/**
 * Rotate a PYRAMID5 element nodes to ensure that the node with the minimum id is the first node
 * for the bottom face.
 * @param min_id_index The index of the node, within the pyramid nodes, with the minimum id for the
 * bottom face
 * @param face_rotation A vector to record the rotation of the faces of the PYRAMID5 element
 * @param node_rotation a vector of node indices that can form a PYRAMID5 element
 */
void nodeRotationPYRAMID5(unsigned int min_id_index,
                          std::vector<unsigned int> & face_rotation,
                          std::vector<unsigned int> & node_rotation);

/**
 * For a rotated nodes that can form a PYRAMID5 element, create a series of four-node set that can
 * form TET4 elements to replace the original PYRAMID5 element. The QUAD4 face of the
 * PYRAMID5 element will be split by the diagonal line that involves the node with the minimum id
 * of that face.
 * @param pyramid_nodes A vector of pointers to the nodes that can form a PYRAMID5 element
 * @param rotated_tet_face_indices A vector of vectors of the original face indices of the
 * PYRAMID5 element corresponding to the faces of the newly created TET4 elements
 * @param tet_nodes_list a vector of vectors of pointers to the nodes that can form TET4 elements
 */
void
pyramidNodesToTetNodesDeterminer(std::vector<const Node *> & pyramid_nodes,
                                 std::vector<std::vector<unsigned int>> & rotated_tet_face_indices,
                                 std::vector<std::vector<const Node *>> & tet_nodes_list);

/**
 * Convert all the elements in a 3D mesh, consisting of only linear elements, into TET4 elements.
 * @param mesh The mesh to be converted
 * @param elems_to_process A vector of pairs of element ids and a bool indicating whether the
 * element needs to be fully retained or will be further processed in the following procedures
 * @param converted_elems_ids_to_track A vector of element ids that need to be tracked for beig
 * further processed in the following procedures
 * @param block_id_to_remove The id of a new subdomain in the mesh containing all the elements to be
 * removed
 * @param delete_block_to_remove A bool indicating whether the block to be removed will be
 * deleted in this method
 */
void convert3DMeshToAllTet4(ReplicatedMesh & mesh,
                            const std::vector<std::pair<dof_id_type, bool>> & elems_to_process,
                            std::vector<dof_id_type> & converted_elems_ids_to_track,
                            const subdomain_id_type block_id_to_remove,
                            const bool delete_block_to_remove);

/**
 * Convert all the elements in a 3D mesh consisting of only linear elements into TET4 elements.
 * @param mesh The mesh to be converted
 */
void convert3DMeshToAllTet4(ReplicatedMesh & mesh);

/**
 * Collect the boundary information of the given element in a mesh.
 * @param bdry_side_list A list that contains the boundary information of the mesh
 * @param elem_id The id of the element to be processed
 * @param n_elem_sides The number of sides of the element
 * @param elem_side_list a vector of vectors to record the boundary information of the element
 */
void
elementBoundaryInfoCollector(const std::vector<libMesh::BoundaryInfo::BCTuple> & bdry_side_list,
                             const dof_id_type elem_id,
                             const unsigned short n_elem_sides,
                             std::vector<std::vector<boundary_id_type>> & elem_side_list);
}
