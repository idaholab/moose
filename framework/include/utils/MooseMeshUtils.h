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

std::vector<libMesh::boundary_id_type>
getBoundaryIDs(const libMesh::MeshBase & mesh,
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
}
