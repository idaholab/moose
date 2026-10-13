//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/mesh_base.h"
#include "libmesh/replicated_mesh.h"
#include "libmesh/point_locator_base.h"

#include "MooseTypes.h"

namespace MooseMeshXYMeshCuttingUtils
{
/// Mode for the mesh-by-mesh cutter
enum class CutMode
{
  REMOVE_INSIDE = 0,
  REMOVE_OUTSIDE = 1,
  KEEP_BOTH = 2
};

/**
 * Build an ordered closed-polyline from a 2D mesh's outer boundary. The cutter mesh's outer
 * boundary must form a single simple closed loop. The returned polyline is oriented CCW (positive
 * signed area). The polyline is closed implicitly: the last point connects back to the first.
 *
 * @param cutter cutter mesh
 * @return ordered vector of Points along the outer boundary
 */
std::vector<Point> extractClosedOuterPolyline(const libMesh::MeshBase & cutter);

/**
 * Classify a 2D point against a closed polyline (using the cutter mesh's PointLocator for the
 * inside test, and an explicit on-polyline tolerance for the boundary).
 *
 * @param p the point to classify
 * @param cutter_locator a PointLocator built on the cutter mesh (out_of_mesh_mode enabled)
 * @param polyline the closed cutter polyline
 * @param on_tol distance tolerance for classifying a point as on the polyline
 * @return -1 outside, 0 on polyline, +1 inside
 */
short classifyPointVsPolyline(const Point & p,
                              libMesh::PointLocatorBase & cutter_locator,
                              const std::vector<Point> & polyline,
                              Real on_tol);

/// A single intersection between a primary edge and the cutter polyline.
struct EdgePolylineHit
{
  Real t;             // parameter along (a -> b) edge in [0, 1]
  std::size_t seg;    // cutter polyline segment index
  Point p;            // intersection point
};

/**
 * Compute all intersection points of a primary edge (a -> b) with the cutter polyline, sorted by
 * the parameter t along the edge.
 *
 * @param a edge start point
 * @param b edge end point
 * @param polyline closed cutter polyline
 * @param tol tolerance for intersection / collinearity detection
 * @return sorted list of EdgePolylineHits in t-ascending order
 */
std::vector<EdgePolylineHit> intersectEdgeWithPolyline(const Point & a,
                                                       const Point & b,
                                                       const std::vector<Point> & polyline,
                                                       Real tol);

/**
 * Snap primary mesh nodes onto a closed cutter polyline. A primary node within snap_tol of a
 * cutter polyline vertex is moved to that vertex; otherwise, if within snap_tol of a cutter
 * polyline segment, it is projected onto that segment.
 *
 * @param primary primary mesh, modified in place
 * @param polyline closed cutter polyline
 * @param snap_tol snap distance tolerance (no-op if 0)
 * @param only_interior if true, only primary nodes not on the primary mesh's external boundary
 *                      are eligible for snapping
 * @return number of primary nodes that were moved
 */
std::size_t snapNodesToPolyline(libMesh::ReplicatedMesh & primary,
                                const std::vector<Point> & polyline,
                                Real snap_tol,
                                bool only_interior);

/**
 * Core clipper: replace primary elements crossed by the cutter polyline with C0POLYGON
 * element(s), inheriting boundary ids per side from the original element and tagging the new
 * cut edges with new_boundary_id. Fully-removed primary elements (no retained vertices, for the
 * remove modes) are marked with block_id_to_remove for deletion.
 *
 * Throws MooseException if a primary element is split into multiple disconnected retained
 * components (the cutter polyline weaves in and out of a single primary element).
 *
 * @param primary primary mesh, modified in place
 * @param polyline closed cutter polyline (CCW)
 * @param cutter_locator a PointLocator on the cutter mesh (out_of_mesh_mode enabled)
 * @param mode CutMode (REMOVE_INSIDE, REMOVE_OUTSIDE, or KEEP_BOTH)
 * @param outside_shift subdomain id shift applied to elements/pieces on the outside of the
 *                      cutter (used for REMOVE_INSIDE and KEEP_BOTH; ignored for REMOVE_OUTSIDE)
 * @param inside_shift subdomain id shift applied to elements/pieces on the inside of the cutter
 *                     (used for REMOVE_OUTSIDE and KEEP_BOTH; ignored for REMOVE_INSIDE)
 * @param outside_suffix subdomain name suffix for the outside piece
 * @param inside_suffix subdomain name suffix for the inside piece
 * @param block_id_to_remove temporary subdomain id used to mark removed elements
 * @param new_boundary_id boundary id assigned to the new cut interface
 * @param tol tolerance for geometric tests
 */
void meshCutterRemoverCutElemPoly(libMesh::ReplicatedMesh & primary,
                                  const std::vector<Point> & polyline,
                                  libMesh::PointLocatorBase & cutter_locator,
                                  CutMode mode,
                                  subdomain_id_type outside_shift,
                                  subdomain_id_type inside_shift,
                                  const SubdomainName & outside_suffix,
                                  const SubdomainName & inside_suffix,
                                  subdomain_id_type block_id_to_remove,
                                  boundary_id_type new_boundary_id,
                                  Real tol);
}
