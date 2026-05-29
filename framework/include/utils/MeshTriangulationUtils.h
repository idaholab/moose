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

#include "MooseTypes.h"
#include "MeshGenerator.h"

namespace MeshTriangulationUtils
{

/**
 * Bundle of inputs for triangulateWithDelaunay. Mirrors the user-facing parameter set of
 * XYDelaunayGenerator, plus parent-class fields for the auto area function. Empty optional fields
 * are skipped.
 */
struct XYDelaunayOptions
{
  std::vector<BoundaryName> input_boundary_names;
  std::vector<SubdomainName> input_subdomain_names;
  unsigned int add_nodes_per_boundary_segment = 0;
  bool refine_bdy = true;
  bool verify_holes = true;
  bool smooth_tri = false;
  Real desired_area = 0;
  std::string desired_area_func;
  bool use_auto_area_func = false;
  Real auto_area_func_default_size = 0;
  Real auto_area_func_default_size_dist = 0;
  unsigned int auto_area_function_num_points = 0;
  Real auto_area_function_power = 0;
  std::vector<Point> interior_points;
  std::string tri_elem_type = "DEFAULT"; // TRI3, TRI6, TRI7, or DEFAULT
  std::vector<bool> stitch_holes;
  std::vector<bool> refine_holes;
  bool use_binary_search = true;
  bool verbose_stitching = false;
  bool has_output_subdomain_id = false;
  SubdomainID output_subdomain_id = 0;
  bool has_output_subdomain_name = false;
  SubdomainName output_subdomain_name;
  bool has_output_boundary = false;
  BoundaryName output_boundary;
  std::vector<BoundaryName> hole_boundaries; // empty if not set
  // Optional per-hole bid filters to help with MeshedHole construction.
  // Used then a boundary layer is added to a hole. MeshedHole will auto-detect if this is empty.
  std::vector<std::set<std::size_t>> hole_boundary_id_filters;
  // Since we need to assign the innermost boundary of the hole boundary layer mesh, we need to have
  // a record of its default bcid.
  std::vector<std::set<BoundaryID>> hole_boundary_inner_id_defaults;
};

/**
 * Performs a 2D Delaunay triangulation (via libMesh::Poly2TriTriangulator) inside a closed boundary
 * mesh with optional holes, optionally stitches the resulting triangulation to each hole mesh that
 * requests it, and applies subdomain / boundary renumbering. This is the core algorithm shared by
 * XYDelaunayGenerator and XYTriangleBoundaryLayerGenerator.
 * @param mg The calling mesh generator (used for paramError reporting and communicator)
 * @param boundary_mesh The closed-loop boundary mesh
 * @param hole_meshes Optional hole meshes (one mesh per hole; empty vector if no holes)
 * @param xyd_opts Triangulation options (see XYDelaunayOptions)
 * @return The triangulated mesh with optionally stitched holes
 */
std::unique_ptr<MeshBase>
triangulateWithDelaunay(MeshGenerator & mg,
                        std::unique_ptr<MeshBase> boundary_mesh,
                        std::vector<std::unique_ptr<MeshBase>> hole_meshes,
                        const XYDelaunayOptions & xyd_opts);
}
