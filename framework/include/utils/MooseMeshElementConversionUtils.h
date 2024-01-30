//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/mesh_base.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/boundary_info.h"

#include "MooseUtils.h"
#include "MooseTypes.h"
#include "FaceInfo.h"

namespace MooseMeshElementConversionUtils
{
/**
 * Split a HEX8 element into six TET4 elements.
 * @param mesh The mesh to be modified
 * @param elem_id The id of the element to be split
 * @param converted_elems_ids a vector to record the ids of the newly created TET4 elements
 */
void hexElemSplitter(ReplicatedMesh & mesh,
                     const dof_id_type elem_id,
                     std::vector<dof_id_type> & converted_elems_ids);

/**
 * Split a PYRAMID5 element into two TET4 elements.
 * @param mesh The mesh to be modified
 * @param elem_id The id of the element to be split
 * @param converted_elems_ids a vector to record the ids of the newly created TET4 elements
 */
void pyramidElemSplitter(ReplicatedMesh & mesh,
                         const dof_id_type elem_id,
                         std::vector<dof_id_type> & converted_elems_ids);

/**
 * Split a PRISM6 element into three TET4 elements.
 * @param mesh The mesh to be modified
 * @param elem_id The id of the element to be split
 * @param converted_elems_ids a vector to record the ids of the newly created TET4 elements
 */
void prismElemSplitter(ReplicatedMesh & mesh,
                       const dof_id_type elem_id,
                       std::vector<dof_id_type> & converted_elems_ids);

/**
 * Rotate a HEX8 element's nodes to ensure that the node with the minimum id is the first node;
 * and the node among its three neighboring nodes with the minimum id is the second node.
 * @param min_id_index The index of the node with the minimum id
 * @param sec_min_pos The position of the node among its three neighboring nodes with the minimum
 * id
 * @param face_rotation A vector to record the rotation of the faces of the HEX8 element
 */
std::vector<unsigned int> nodeRotationHEX8(unsigned int min_id_index,
                                           unsigned int sec_min_pos,
                                           std::vector<unsigned int> & face_rotation);

/**
 * Calculate the three neighboring nodes of a node in a HEX8 element.
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
 * @return a vector of vectors of pointers to the nodes that can form TET4 elements
 */
std::vector<std::vector<Node *>>
hexNodesToTetNodesDeterminer(std::vector<Node *> & hex_nodes,
                             std::vector<std::vector<unsigned int>> & rotated_tet_face_indices);

/**
 * For a HEX8 element, determine the direction of the diagonal line of each face that involves the
 * node with the minimum id of that face.
 * @param hex_nodes A vector of pointers to the nodes that can form a HEX8 element
 * @return a vector of boolean values indicating the direction of the diagonal line of each face
 */
std::vector<bool> quadFaceDiagonalDirectionsHex(const std::vector<Node *> & hex_nodes);

/**
 * For a QUAD4 element, determine the direction of the diagonal line that involves the node with
 * the minimum id of that element.
 * @param quad_nodes A vector of pointers to the nodes that can form a QUAD4 element
 * @return a boolean value indicating the direction of the diagonal line
 */
bool quadFaceDiagonalDirection(const std::vector<Node *> & quad_nodes);

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
 * @param min_id_index The index of the node with the minimum id
 * @param face_rotation A vector to record the rotation of the faces of the PRISM6 element
 * @return a vector of node indices that can form a PRISM6 element
 */
std::vector<unsigned int> nodeRotationPRISM6(unsigned int min_id_index,
                                             std::vector<unsigned int> & face_rotation);

/**
 * For a rotated nodes that can form a PRISM6 element, create a series of four-node set that can
 * form TET4 elements to replace the original PRISM6 element. All the QUAD4 face of the PRISM6
 * element will be split by the diagonal line that involves the node with the minimum id of that
 * face.
 * @param prism_nodes A vector of pointers to the nodes that can form a PRISM6 element
 * @param rotated_tet_face_indices A vector of vectors of the original face indices of the PRISM6
 * element corresponding to the faces of the newly created TET4 elements
 * @return a vector of vectors of pointers to the nodes that can form TET4 elements
 */
std::vector<std::vector<Node *>>
prismNodesToTetNodesDeterminer(std::vector<Node *> & prism_nodes,
                               std::vector<std::vector<unsigned int>> & rotated_tet_face_indices);

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
 * @param min_id_index The index of the node with the minimum id for the bottom face
 * @param face_rotation A vector to record the rotation of the faces of the PYRAMID5 element
 * @return a vector of node indices that can form a PYRAMID5 element
 */
std::vector<unsigned int> nodeRotationPYRAMIND5(unsigned int min_id_index,
                                                std::vector<unsigned int> & face_rotation);

/**
 * For a rotated nodes that can form a PYRAMID5 element, create a series of four-node set that can
 * form TET4 elements to replace the original PYRAMID5 element. The QUAD4 face of the
 * PYRAMID5 element will be split by the diagonal line that involves the node with the minimum id
 * of that face.
 * @param pyramid_nodes A vector of pointers to the nodes that can form a PYRAMID5 element
 * @param rotated_tet_face_indices A vector of vectors of the original face indices of the
 * PYRAMID5 element corresponding to the faces of the newly created TET4 elements
 * @return a vector of vectors of pointers to the nodes that can form TET4 elements
 */
std::vector<std::vector<Node *>>
pyramidNodesToTetNodesDeterminer(std::vector<Node *> & pyramid_nodes,
                                 std::vector<std::vector<unsigned int>> & rotated_tet_face_indices);

/**
 * Convert all the elements in a 3D mesh consisting only linear elements into TET4 elements.
 * @param mesh The mesh to be converted
 * @param elems_to_process A vector of pairs of element ids and a bool indicating whether the
 * element needs to be fully retained or will be partially cut in the following procedures
 * @param converted_elems_ids_to_cut A vector of element ids that will be cut in the following
 * procedures
 * @param converted_elems_ids_to_retain A vector of element ids that will be fully retained in
 * the following procedures
 * @param block_id_to_remove The id of a new subdomain in the mesh containing all the elements to be
 * removed
 * @param delete_block_to_remove A bool indicating whether the block to be removed will be
 * deleted in this method
 */
void convert3DMeshToAllTet4(ReplicatedMesh & mesh,
                            const std::vector<std::pair<dof_id_type, bool>> & elems_to_process,
                            std::vector<dof_id_type> & converted_elems_ids_to_cut,
                            std::vector<dof_id_type> & converted_elems_ids_to_retain,
                            const subdomain_id_type block_id_to_remove,
                            const bool delete_block_to_remove);

/**
 * Convert all the elements in a 3D mesh consisting only linear elemetns into TET4 elements.
 * @param mesh The mesh to be converted
 */
void convert3DMeshToAllTet4(ReplicatedMesh & mesh);
}
