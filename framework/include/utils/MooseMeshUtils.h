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

namespace MooseMeshUtils
{
/**
 * Changes the old boundary ID to a new ID in the mesh
 *
 * @param mesh the mesh
 * @param old_id the old boundary id
 * @param new_id the new boundary id
 * @param delete_prev whether to delete the previous boundary id from the mesh
 */
void changeBoundaryId(MeshBase & mesh,
                      const libMesh::boundary_id_type old_id,
                      const libMesh::boundary_id_type new_id,
                      bool delete_prev);

/**
 * Changes the old subdomain ID to a new ID in the mesh
 *
 * @param mesh the mesh
 * @param old_id the old subdomain id
 * @param new_id the new subdomain id
 */
void
changeSubdomainId(MeshBase & mesh, const subdomain_id_type old_id, const subdomain_id_type new_id);

/**
 * Gets the boundary IDs with their names.
 *
 * The ordering of the returned boundary ID vector matches the vector of the boundary
 * names in \p boundary_name.
 * When a boundary name is not available in the mesh, if \p generate_unknown is true
 * a non-existant boundary ID will be returned, otherwise a BoundaryInfo::invalid_id
 * will be returned.
 */
std::vector<BoundaryID> getBoundaryIDs(const libMesh::MeshBase & mesh,
                                       const std::vector<BoundaryName> & boundary_name,
                                       bool generate_unknown,
                                       const std::set<BoundaryID> & mesh_boundary_ids);

/**
 * Gets the boundary IDs with their names.
 *
 * The ordering of the returned boundary ID vector matches the vector of the boundary
 * names in \p boundary_name.
 * When a boundary name is not available in the mesh, if \p generate_unknown is true
 * a non-existant boundary ID will be returned, otherwise a BoundaryInfo::invalid_id
 * will be returned.
 */
std::vector<BoundaryID> getBoundaryIDs(const libMesh::MeshBase & mesh,
                                       const std::vector<BoundaryName> & boundary_name,
                                       bool generate_unknown);

/**
 * Gets the boundary IDs into a set with their names.
 *
 * Because libMesh allows the same boundary to have multiple different boundary names,
 * the size of the returned boundary ID set may be smaller than the size of the bounndary
 * name vector.
 * When a boundary name is not available in the mesh, if \p generate_unknown is true
 * a non-existant boundary ID will be returned, otherwise a BoundaryInfo::invalid_id
 * will be returned.
 */
std::set<BoundaryID> getBoundaryIDSet(const libMesh::MeshBase & mesh,
                                      const std::vector<BoundaryName> & boundary_name,
                                      bool generate_unknown);

/**
 * Gets the boundary ID associated with the given BoundaryName.
 *
 * This is needed because the BoundaryName can be either an ID or a name.
 * If it is a name, the mesh is queried for the ID associated with said name.
 */
BoundaryID getBoundaryID(const BoundaryName & boundary_name, const MeshBase & mesh);

/**
 * Gets the subdomain ID associated with the given SubdomainName.
 *
 * This is needed because the SubdomainName can be either an ID or a name.
 * If it is a name, the mesh is queried for the ID associated with said name.
 */
SubdomainID getSubdomainID(const SubdomainName & subdomain_name, const MeshBase & mesh);

/**
 * Get the associated subdomainIDs for the subdomain names that are passed in.
 *
 * @param mesh The mesh
 * @param subdomain_name The names of the subdomains
 * @return The subdomain ids from the passed subdomain names.
 */
std::vector<subdomain_id_type> getSubdomainIDs(const libMesh::MeshBase & mesh,
                                               const std::vector<SubdomainName> & subdomain_name);

/**
 * Calculates the centroid of a MeshBase.
 * @param mesh input mesh whose centroid needs to be calculated
 * @return a Point data containing the mesh centroid
 */
Point meshCentroidCalculator(const MeshBase & mesh);

/**
 * compute a coordinate transformation factor
 * @param point The libMesh \p Point in space where we are evaluating the factor
 * @param factor The output of this function. Would be 1 for cartesian coordinate systems, 2*pi*r
 * for cylindrical coordinate systems, and 4*pi*r^2 for spherical coordinate systems
 * @param coord_type The coordinate system type, e.g. cartesian (COORD_XYZ), cylindrical (COORD_RZ),
 * or spherical (COORD_RSPHERICAL)
 * @param rz_radial_coord The index at which to index \p point for the radial coordinate when in a
 * cylindrical coordinate system
 */
template <typename P, typename C>
void
coordTransformFactor(const P & point,
                     C & factor,
                     const Moose::CoordinateSystemType coord_type,
                     const unsigned int rz_radial_coord = libMesh::invalid_uint)
{
  switch (coord_type)
  {
    case Moose::COORD_XYZ:
      factor = 1.0;
      break;
    case Moose::COORD_RZ:
    {
      mooseAssert(rz_radial_coord != libMesh::invalid_uint,
                  "Must pass in a valid rz radial coordinate");
      factor = 2 * M_PI * point(rz_radial_coord);
      break;
    }
    case Moose::COORD_RSPHERICAL:
      factor = 4 * M_PI * point(0) * point(0);
      break;
    default:
      mooseError("Unknown coordinate system");
  }
}

/**
 * Computes the distance to a general axis
 *
 * @param[in] point  Point for which to compute distance from axis
 * @param[in] origin  Axis starting point
 * @param[in] direction  Axis direction
 */
template <typename P, typename C>
C
computeDistanceToAxis(const P & point, const Point & origin, const RealVectorValue & direction)
{
  return (point - origin).cross(direction).norm();
}

/**
 * Computes a coordinate transformation factor for a general axisymmetric axis
 *
 * @param[in] point  The libMesh \p Point in space where we are evaluating the factor
 * @param[in] axis  The pair of values defining the general axisymmetric axis.
 *                  Respectively, the values are the axis starting point and direction.
 * @param[out] factor  The coordinate transformation factor
 */
template <typename P, typename C>
void
coordTransformFactorRZGeneral(const P & point,
                              const std::pair<Point, RealVectorValue> & axis,
                              C & factor)
{
  factor = 2 * M_PI * computeDistanceToAxis<P, C>(point, axis.first, axis.second);
}

inline void
computeFaceInfoFaceCoord(FaceInfo & fi,
                         const Moose::CoordinateSystemType coord_type,
                         const unsigned int rz_radial_coord = libMesh::invalid_uint)
{
  coordTransformFactor(fi.faceCentroid(), fi.faceCoord(), coord_type, rz_radial_coord);
}

/**
 * Crate a new set of element-wise IDs by finding unique combinations of existing extra ID values
 *
 * This function finds the unique combinations by recursively calling itself for extra ID inputs. In
 * the recursive calling, the new unique combinations is determined by combining the extra ID value
 * of current level and the unique combination determined in the previous level in recursion. In the
 * lowest level of recursion, the base combination is set by the unique ID values of the
 * corresponding extra ID.
 *
 * @param mesh input mesh
 * @param block_ids block ids
 * @param extra_ids extra ids
 * @return map of element id to new extra id
 **/
std::unordered_map<dof_id_type, dof_id_type>
getExtraIDUniqueCombinationMap(const MeshBase & mesh,
                               const std::set<SubdomainID> & block_ids,
                               std::vector<ExtraElementIDName> extra_ids);

/**
 * Decides whether all the Points of a vector of Points are in a plane that is defined by a normal
 * vector and an inplane Point
 * @param vec_pts vector of points to be examined
 * @param plane_nvec normal vector of the plane
 * @param fixed_pt a Point in the plane
 * @return whether all the Points are in the given plane
 */
bool isCoPlanar(const std::vector<Point> vec_pts, const Point plane_nvec, const Point fixed_pt);

/**
 * Decides whether all the Points of a vector of Points are in a plane with a given normal vector
 * @param vec_pts vector of points to be examined
 * @param plane_nvec normal vector of the plane
 * @return whether all the Points are in the same plane with the given normal vector
 */
bool isCoPlanar(const std::vector<Point> vec_pts, const Point plane_nvec);

/**
 * Decides whether all the Points of a vector of Points are coplanar
 * @param vec_pts vector of points to be examined
 * @return whether all the Points are in a same plane
 */
bool isCoPlanar(const std::vector<Point> vec_pts);

/**
 * Checks input mesh and returns max(block ID) + 1, which represents
 * a block ID that is not currently in use in the mesh
 * @param input mesh over which to compute the next free block id
 */
SubdomainID getNextFreeSubdomainID(MeshBase & input_mesh);

/**
 * Checks input mesh and returns the largest boundary ID in the mesh plus one, which is
 * a boundary ID in the mesh that is not currently in use
 * @param input mesh over which to compute the next free boundary ID
 */

BoundaryID getNextFreeBoundaryID(MeshBase & input_mesh);
/**
 * Whether a particular subdomain ID exists in the mesh
 * @param input mesh over which to determine subdomain IDs
 * @param subdomain ID
 */
bool hasSubdomainID(MeshBase & input_mesh, const SubdomainID & id);

/**
 * Whether a particular subdomain name exists in the mesh
 * @param input mesh over which to determine subdomain names
 * @param subdomain name
 */
bool hasSubdomainName(MeshBase & input_mesh, const SubdomainName & name);

/**
 * Whether a particular boundary ID exists in the mesh
 * @param input mesh over which to determine boundary IDs
 * @param boundary ID
 */
bool hasBoundaryID(const MeshBase & input_mesh, const BoundaryID id);

/**
 * Whether a particular boundary name exists in the mesh
 * @param input mesh over which to determine boundary names
 * @param boundary name
 */
bool hasBoundaryName(const MeshBase & input_mesh, const BoundaryName & name);

/**
 * Convert a list of sides in the form of a vector of pairs of node ids into a list of ordered nodes
 * based on connectivity
 * @param node_assm vector of pairs of node ids that represent the sides
 * @param elem_id_list vector of element ids that represent the elements that contain the sides
 * @param ordered_node_list vector of node ids that represent the ordered nodes
 * @param ordered_elem_id_list vector of element corresponding to the ordered nodes
 * */
void makeOrderedNodeList(std::vector<std::pair<dof_id_type, dof_id_type>> & node_assm,
                         std::vector<dof_id_type> & elem_id_list,
                         std::vector<dof_id_type> & ordered_node_list,
                         std::vector<dof_id_type> & ordered_elem_id_list);

/**
 * Split a HEX8 element into five TET4 element to prepare for the cutting operation.
 * @param mesh The mesh to be modified
 * @param elem_id The id of the element to be split
 * @param converted_elems_ids a vector to record the ids of the newly created TET4 elements
 */
void hexElemSplitter(ReplicatedMesh & mesh,
                     const dof_id_type elem_id,
                     std::vector<dof_id_type> & converted_elems_ids);

/**
 * Split a PYRAMID5 element into two TET4 element to prepare for the cutting operation.
 * @param mesh The mesh to be modified
 * @param elem_id The id of the element to be split
 * @param converted_elems_ids a vector to record the ids of the newly created TET4 elements
 */
void pyramidElemSplitter(ReplicatedMesh & mesh,
                         const dof_id_type elem_id,
                         std::vector<dof_id_type> & converted_elems_ids);

/**
 * Split a PRISM6 element into three TET4 element to prepare for the cutting operation.
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
hexNodeOptimizer(std::vector<Node *> & hex_nodes,
                 std::vector<std::vector<unsigned int>> & rotated_tet_face_indices);

/**
 * For a HEX8 element, determine the direction of the diagonal line of each face that involves the
 * node with the minimum id of that face.
 * @param hex_nodes A vector of pointers to the nodes that can form a HEX8 element
 * @return a vector of boolean values indicating the direction of the diagonal line of each face
 */
std::vector<bool> quadFaceDiagonalDirectionsHex(std::vector<Node *> & hex_nodes);

/**
 * For a QUAD4 element, determine the direction of the diagonal line that involves the node with
 * the minimum id of that element.
 * @param quad_nodes A vector of pointers to the nodes that can form a QUAD4 element
 * @return a boolean value indicating the direction of the diagonal line
 */
bool quadFaceDiagonalDirection(std::vector<Node *> & quad_nodes);

/**
 * Creates sets of four nodes indices that can form TET4 elements to replace the original HEX8
 * element.
 * @param diagonal_directions A vector of boolean values indicating the direction of the diagonal
 * line of each face
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
prismNodeOptimizer(std::vector<Node *> & prism_nodes,
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
pyramidNodeOptimizer(std::vector<Node *> & pyramid_nodes,
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
 * @param block_id_to_remove The id of a new subdomain in the mesh containing all the elements to be removed
 * @param delete_block_to_remove A bool indicating whether the block to be removed will be
 * deleted in this method
 */
void convert3DMeshToAllTet4(ReplicatedMesh & mesh,
                            const std::vector<std::pair<dof_id_type, bool>> & elems_to_process,
                            std::vector<dof_id_type> & converted_elems_ids_to_cut,
                            std::vector<dof_id_type> & converted_elems_ids_to_retain,
                            const subdomain_id_type & block_id_to_remove,
                            const bool delete_block_to_remove);

/**
 * Convert all the elements in a 3D mesh consisting only linear elemetns into TET4 elements.
 * @param mesh The mesh to be converted
 */
void convert3DMeshToAllTet4(ReplicatedMesh & mesh);
}
