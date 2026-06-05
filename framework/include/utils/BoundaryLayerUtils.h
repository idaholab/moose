//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/replicated_mesh.h"
#include "libmesh/mesh_triangle_holes.h"

#include "MooseTypes.h"
#include "MeshGenerator.h"

namespace BoundaryLayerUtils
{

/**
 * Builds a conformal boundary-layer ring of triangulated annuli along a boundary of an input 2D
 * mesh (or a 1D loop). Generates num_layers + 1 parallel polylines (geometric progression of
 * thicknesses with the given bias), triangulates each annulus with triangulateWithDelaunay, and
 * sequentially stitches them. The output mesh carries 2 * num_layers boundary ids, with the
 * innermost being id 1 and the outermost being id (num_layers - 1) * 2.
 * @param mg The calling mesh generator (for paramError reporting + buildMeshBaseObject)
 * @param input_mesh The 2D-XY input mesh or 1D closed loop providing the seed boundary
 * @param boundary_names Subset of boundary names on input_mesh defining the seed boundary; if
 *  empty, the external boundary of input_mesh is auto-detected via MeshedHole
 * @param num_layers Number of element layers to generate
 * @param thickness Total boundary-layer thickness
 * @param layer_bias Geometric growth factor between successive layer thicknesses (1.0 = uniform)
 * @param outward If true, the layer grows outward from the seed boundary; else inward
 * @param tri_elem_type Triangle element type ("TRI3", "TRI6", "TRI7", or "DEFAULT")
 * @param output_subdomain_id Subdomain id assigned to all generated triangles (0 = default)
 * @param output_subdomain_name Subdomain name assigned to output_subdomain_id (empty = unnamed)
 * @return The stitched boundary-layer ring mesh
 */
std::unique_ptr<MeshBase> buildBoundaryLayerRing(MeshGenerator & mg,
                                                 MeshBase & input_mesh,
                                                 const std::vector<BoundaryName> & boundary_names,
                                                 unsigned int num_layers,
                                                 Real thickness,
                                                 Real layer_bias,
                                                 bool outward,
                                                 const MooseEnum & tri_elem_type,
                                                 SubdomainID output_subdomain_id,
                                                 const SubdomainName & output_subdomain_name);

/**
 * Generates a list of points offset from the input boundary polyline by a specified thickness in
 * either outward/inward direction.
 * @param mg The mesh generator calling this function, used for paramError reporting
 * @param ply_mesh_u The 1D loop polyline mesh of the original boundary; the volume it encloses will
 * be triangulated in-place to compute the normal and define the inward/outward directions
 * @param points The vertex points of the original polyline. If empty, populated from the input
 *  mesh via collectExteriorVertexPointsFromMesh.
 * @param mid_points The midpoints of the original polyline (optional to enable quadratic elements).
 * If both this and points are empty, populated from the input mesh.
 * @param outward Whether to offset in the outward (true) or inward (false) direction
 * @param thickness The offset distance (>= 0)
 * @return Offset points: vertices first, then midpoints (length = points.size() +
 * mid_points.size())
 */
std::vector<Point> generateOffsetPolyline(MeshGenerator * mg,
                                          std::unique_ptr<libMesh::UnstructuredMesh> & ply_mesh_u,
                                          std::vector<Point> & points,
                                          std::vector<Point> & mid_points,
                                          const bool outward,
                                          const Real thickness);

/**
 * Collects key vertex points (and optional midpoints) from a meshed hole, optionally discarding
 * colinear vertices.
 * @param bdry_mh The 2D MeshedHole object from which to collect key points
 * @param points The vector to which collected vertex points are appended (in order)
 * @param mid_points The vector to which collected midpoints are appended; only populated when the
 *  MeshedHole reports a midpoint per side AND skip_node_reduction is true
 * @param skip_node_reduction If true, retain every vertex and midpoint; if false, drop vertices
 *  that are colinear with their two neighbors and do not collect midpoints
 */
void collectExteriorVertexPointsFromMesh(libMesh::TriangulatorInterface::MeshedHole & bdry_mh,
                                         std::vector<Point> & points,
                                         std::vector<Point> & mid_points,
                                         const bool skip_node_reduction = false);

/**
 * Extracts the normal vector of the EDGE3 side of a quadratic element at a given node index.
 * @param elem The element containing the side of interest
 * @param s The side index of interest
 * @param node_index The index of the node on the side at which to extract the normal vector
 * @return The normal vector at the node index on the side of interest
 */
Point getKeyNormal(const Elem * elem, const unsigned int s, const unsigned int node_index);
}
