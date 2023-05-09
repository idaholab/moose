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
// Changes the old ID to new ID in the mesh given in parameters
void changeBoundaryId(MeshBase & mesh,
                      const libMesh::boundary_id_type old_id,
                      const libMesh::boundary_id_type new_id,
                      bool delete_prev);

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
 * @param mesh_subdomains All the subdomain IDs that exist on the mesh
 * @return The subdomain ids from the passed subdomain names.
 */
std::vector<subdomain_id_type> getSubdomainIDs(const libMesh::MeshBase & mesh,
                                               const std::vector<SubdomainName> & subdomain_name,
                                               const std::set<SubdomainID> & mesh_subdomains);

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
 * the recursive calling, the new unique combitnations is determined by combining the extra ID value
 * of current level and the unique combination determined in the previous level in recursion. In the
 * lowest level of recursion, the base combination is set by the unqiue ID values of the
 * corresponding extra ID.
 *
 * @param mesh input mesh
 * @param block_ids block ids
 * @param extra_ids extra ids
 * @return map of element id to new extra id
 **/
std::map<dof_id_type, dof_id_type>
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
bool hasBoundaryID(MeshBase & input_mesh, const BoundaryID & id);

/**
 * Whether a particular boundary name exists in the mesh
 * @param input mesh over which to determine boundary names
 * @param boundary name
 */
bool hasBoundaryName(MeshBase & input_mesh, const BoundaryName & name);

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
}
