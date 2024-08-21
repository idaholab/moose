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
#include "libmesh/boundary_info.h"

#include "MooseUtils.h"
#include "MooseTypes.h"
#include "FaceInfo.h"

namespace MooseMeshUtils
{

/**
 * Merges the boundary IDs of boundaries that have the same names
 * but different IDs.
 * @param mesh The input mesh whose boundaries we will modify
 */
void mergeBoundaryIDsWithSameName(MeshBase & mesh);

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
computeFiniteVolumeCoords(FaceInfo & fi,
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
bool hasSubdomainID(const MeshBase & input_mesh, const SubdomainID & id);

/**
 * Whether a particular subdomain name exists in the mesh
 * @param input mesh over which to determine subdomain names
 * @param subdomain name
 */
bool hasSubdomainName(const MeshBase & input_mesh, const SubdomainName & name);

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
 * @param midpoint_node_list vector of node ids that represent the midpoints of the sides for
 * quadratic sides
 * @param ordered_node_list vector of node ids that represent the ordered nodes
 * @param ordered_elem_id_list vector of element corresponding to the ordered nodes
 * */
void makeOrderedNodeList(std::vector<std::pair<dof_id_type, dof_id_type>> & node_assm,
                         std::vector<dof_id_type> & elem_id_list,
                         std::vector<dof_id_type> & midpoint_node_list,
                         std::vector<dof_id_type> & ordered_node_list,
                         std::vector<dof_id_type> & ordered_elem_id_list);

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
 * Converts a given name (BoundaryName or SubdomainName) that is known to only contain digits into a
 * corresponding ID (BoundaryID or SubdomainID) and performs bounds checking to ensure that overflow
 * doesn't happen.
 * @param name Name that is to be converted into an ID.
 * @return ID type corresponding to the type of name.
 */
template <typename T, typename Q>
Q
getIDFromName(const T & name)
{
  if (!MooseUtils::isDigits(name))
    mooseError(
        "'name' ", name, " should only contain digits that can be converted to a numerical type.");
  long long id = std::stoll(name);
  Q id_Q = Q(id);
  if (id < std::numeric_limits<Q>::min() || id > std::numeric_limits<Q>::max())
    mooseError(MooseUtils::prettyCppType<T>(&name),
               " ",
               name,
               " is not within the numeric limits of the expected ID type ",
               MooseUtils::prettyCppType<Q>(&id_Q),
               ".");

  return id_Q;
}

/**
 * Swap two nodes within an element
 * @param elem element whose nodes need to be swapped
 * @param nd1 index of the first node to be swapped
 * @param nd2 index of the second node to be swapped
 */
void swapNodesInElem(Elem & elem, const unsigned int nd1, const unsigned int nd2);

/**
 * Reprocess the swap related input parameters to make pairs out of them to ease further processing
 * @param class_name name of the mesh generator class used for exception messages
 * @param id_name name of the parameter to be swapped used for exception messages
 * @param id_swaps vector of vectors of the ids to be swapped
 * @param id_swap_pairs vector of maps of the swapped pairs
 * @param row_index_shift shift to be applied to the row index in the exception messages (useful
 * when this method is utilized to process a fraction of a long vector)
 */
template <typename T>
void
idSwapParametersProcessor(const std::string & class_name,
                          const std::string & id_name,
                          const std::vector<std::vector<T>> & id_swaps,
                          std::vector<std::unordered_map<T, T>> & id_swap_pairs,
                          const unsigned int row_index_shift = 0)
{
  id_swap_pairs.resize(id_swaps.size());
  for (const auto i : index_range(id_swaps))
  {
    const auto & swaps = id_swaps[i];
    auto & swap_pairs = id_swap_pairs[i];

    if (swaps.size() % 2)
      throw MooseException("Row ",
                           row_index_shift + i + 1,
                           " of ",
                           id_name,
                           " in ",
                           class_name,
                           " does not contain an even number of entries! Num entries: ",
                           swaps.size());

    swap_pairs.reserve(swaps.size() / 2);
    for (unsigned int j = 0; j < swaps.size(); j += 2)
      swap_pairs[swaps[j]] = swaps[j + 1];
  }
}

/**
 * Reprocess the elem_integers_swaps into maps so they are easier to use
 * @param class_name name of the mesh generator class used for exception messages
 * @param num_sections number of sections in the mesh
 * @param num_integers number of extra element integers in the mesh
 * @param elem_integers_swaps vector of vectors of vectors of extra element ids to be swapped
 * @param elem_integers_swap_pairs vector of maps of the swapped pairs
 */
void extraElemIntegerSwapParametersProcessor(
    const std::string & class_name,
    const unsigned int num_sections,
    const unsigned int num_integers,
    const std::vector<std::vector<std::vector<dof_id_type>>> & elem_integers_swaps,
    std::vector<std::unordered_map<dof_id_type, dof_id_type>> & elem_integers_swap_pairs);
}
