//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SurfaceSubdomainsDelaunayRemesher.h"

#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/boundary_info.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/unstructured_mesh.h"

#include "libmesh/mesh_base.h"
#include "libmesh/replicated_mesh.h"

registerMooseObject("MooseApp", SurfaceSubdomainsDelaunayRemesher);

InputParameters
SurfaceSubdomainsDelaunayRemesher::validParams()
{
  InputParameters params = SurfaceDelaunayGeneratorBase::validParams();
  params += LevelSetMeshingHelper::validParams();
  params.renameParameterGroup("Parsed expression advanced", "Level set shape parsed expression");

  params.addClassDescription(
      "Mesh generator that re-meshes a 2D surface mesh given as one or more subdomains into "
      "a 2D surface mesh using Delaunay triangulation.");

  // Definition of the region to re-mesh
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addParam<std::vector<std::vector<SubdomainName>>>(
      "subdomain_names", {}, "The surface mesh subdomains to be re-meshed");
  params.addParam<std::vector<SubdomainName>>(
      "exclude_subdomain_names", {}, "The surface mesh subdomains that should not be re-meshed");

  // Surface re-meshing parameters
  params.addRangeCheckedParam<Real>(
      "max_edge_length",
      "max_edge_length>0",
      "Maximum length of an edge in the 1D meshes around each subdomain. Only a single value is "
      "currently allowed as the boundary refinement must be consistent between all subdomains.");
  // NOTE: we could make this a per-included-subdomain parameter, we would just need to identify
  // interfaces between each subdomain and use the minimum (or average or maximum, just needs to
  // be consistent) edge length on those interfaces.
  params.addParam<std::vector<unsigned int>>(
      "interpolate_boundaries",
      {1},
      "Ratio of points to add to the boundaries. Can be set to a single value for all groups at "
      "once, or specified individually. A single value is recommended as the boundary refinement "
      "must be consistent between all subdomains.");
  params.addParam<std::vector<Real>>(
      "desired_areas",
      {0},
      "Target element size when triangulating projection of the subdomain group. Can be set to a "
      "single value for all groups at once, or specified individually. Default of 0 means no "
      "constraint.");
  params.addRangeCheckedParam<Real>(
      "max_angle_deviation",
      60.0,
      "max_angle_deviation>0 & max_angle_deviation<90",
      "Maximum angle deviation from the average normal vector in each group of subdomains.");
  params.addParamNamesToGroup(
      "max_edge_length interpolate_boundaries desired_areas max_angle_deviation",
      "Delaunay triangulation");

  // Parameters for stitching meshes at the end
  params.addParam<bool>("avoid_merging_subdomains",
                        false,
                        "Whether to prevent merging subdomains by offsetting ids. The first mesh "
                        "in the input will keep the same subdomains ids, the others will have "
                        "offsets. All subdomain names will remain valid");
  params.addParam<bool>("clear_stitching_boundaries",
                        true,
                        "Whether to clear the boundaries between the subdomains being stitched "
                        "together after they were re-meshed with triangles");
  MooseEnum algorithm("BINARY EXHAUSTIVE", "BINARY");
  params.addParam<MooseEnum>(
      "stitching_algorithm",
      algorithm,
      "Control the use of binary search for the nodes of the stitched surfaces.");
  params.addParam<bool>(
      "verbose_stitching", false, "Whether mesh stitching should have verbose output.");
  params.addParamNamesToGroup(
      "avoid_merging_subdomains clear_stitching_boundaries stitching_algorithm verbose_stitching",
      "Stitching triangularized meshes");

  params.addParam<bool>(
      "verbose", false, "Whether the generator should output additional information");

  return params;
}

SurfaceSubdomainsDelaunayRemesher::SurfaceSubdomainsDelaunayRemesher(
    const InputParameters & parameters)
  : SurfaceDelaunayGeneratorBase(parameters),
    LevelSetMeshingHelper(parameters),
    _input(getMesh("input")),
    _subdomain_names(getParam<std::vector<std::vector<SubdomainName>>>("subdomain_names")),
    _max_level_set_correction_iterations(
        getParam<unsigned int>("max_level_set_correction_iterations")),
    _max_angle_deviation(getParam<Real>("max_angle_deviation")),
    _interpolate_boundaries(getParam<std::vector<unsigned int>>("interpolate_boundaries")),
    _desired_areas(getParam<std::vector<Real>>("desired_areas")),
    _verbose(getParam<bool>("verbose"))
{
  if (isParamSetByUser("subdomain_names") && isParamSetByUser("exclude_subdomain_names"))
    paramError("exclude_subdomain_names",
               "Excluding subdomain names is only to be set when 'subdomain_names' is not set");
}

std::unique_ptr<MeshBase>
SurfaceSubdomainsDelaunayRemesher::generate()
{
  std::unique_ptr<MeshBase> mesh_3d = std::move(_input);

  // Select subdomain names if it has not been selected by the user
  // We form 1 group per subdomain by default (easiest to mesh)
  if (_subdomain_names.empty() || _subdomain_names[0].empty())
  {
    if (_subdomain_names.size())
      _subdomain_names.erase(_subdomain_names.begin());

    // Get all subdomains from the mesh
    std::set<subdomain_id_type> sub_ids;
    mesh_3d->subdomain_ids(sub_ids);

    // Copy only the ones that are not excluded
    const auto & excluded = getParam<std::vector<SubdomainName>>("exclude_subdomain_names");
    for (const auto & sub_id : sub_ids)
    {
      const auto & sn = mesh_3d->subdomain_name(sub_id) == "" ? std::to_string(sub_id)
                                                              : mesh_3d->subdomain_name(sub_id);
      if (excluded.empty() || std::find(excluded.begin(), excluded.end(), sn) == excluded.end())
        _subdomain_names.push_back({sn});
    }
  }
  _num_groups = _subdomain_names.size();
  if (_interpolate_boundaries.size() == 1)
    _interpolate_boundaries.resize(_num_groups, _interpolate_boundaries[0]);
  if (_desired_areas.size() == 1)
    _desired_areas.resize(_num_groups, _desired_areas[0]);
  if (_interpolate_boundaries.size() != _num_groups)
    paramError("interpolate_boundaries",
               "Should be the same size as 'subdomain_names' or of size 1");
  if (_desired_areas.size() != _num_groups)
    paramError("desired_areas", "Should be the same size as 'subdomain_names' or of size 1");

  // Re-create surface meshes from each of the subdomains
  std::vector<std::unique_ptr<ReplicatedMesh>> remeshed_2d;

  for (const auto i : make_range(_num_groups))
  {
    auto mesh_2d = buildReplicatedMesh();
    MooseMeshUtils::convertBlockToMesh(*mesh_3d, *mesh_2d, _subdomain_names[i]);

    // Mesh the subdomains by groups
    // TODO: holes for each subdomain are not currently supported
    std::vector<std::unique_ptr<ReplicatedMesh>> no_holes = {};
    auto new_mesh = General2DDelaunay(mesh_2d, /*holes*/ no_holes, i);

    // Keep the remeshed subdomains 2D meshes
    remeshed_2d.push_back(std::move(new_mesh));
  }

  // TODO: can we gain efficiency by using different boundary ids for each stitch?
  const auto primary_bcid = 1;
  // Prevent deleting a legitimate ID if we start conserving boundary IDs from the input mesh
  // to the output mesh
  const auto starting_stitching_id = MooseMeshUtils::getNextFreeBoundaryID(*mesh_3d);
  auto paired_bcid = starting_stitching_id;
  const auto verbose_stitching = getParam<bool>("verbose_stitching");
  const auto use_binary_search = getParam<MooseEnum>("stitching_algorithm") == "BINARY";

  // Stitch all the parts to the first one
  std::unique_ptr<ReplicatedMesh> full_mesh;
  full_mesh = std::move(remeshed_2d[0]);

  // Build a subdomain map
  auto & main_subdomain_map = full_mesh->set_subdomain_name_map();

  for (auto remesh_i : index_range(remeshed_2d))
  {
    if (remesh_i == 0)
      continue;

    UnstructuredMesh & remeshed = dynamic_cast<UnstructuredMesh &>(*remeshed_2d[remesh_i]);

    // NOTE: we cannot use MeshedHole because the meshes are no longer in the XY plane
    // Potential optimization: instead of using the full mesh outer boundary for stitching
    // use what is done in the XYDelaunayGenerator and re-build only the boundary near the
    // mesh that we are stitching
    // Potential optimization: we could keep the external boundaries from the triangulation phase

    // Create the boundary outside the remeshed mesh
    bool rem_has_external_bdy = false;
    MooseMeshUtils::addExternalBoundary(remeshed, paired_bcid, rem_has_external_bdy);
    mooseAssert(rem_has_external_bdy, "Subdomain mesh should have an external boundary");

    // We need to re-create the boundary outside the parent mesh because we are clearing
    // it on every stitch
    bool base_has_external_bdy = false;
    MooseMeshUtils::addExternalBoundary(*full_mesh, primary_bcid, base_has_external_bdy);
    if (!base_has_external_bdy)
      mooseWarning(
          "Full mesh should have an external boundary. This could occur if the surface mesh is "
          "disconnected in several parts and a part's surface mesh has just been fully remeshed");

    // Retrieve subdomain name map from the mesh to be stitched and insert it into the main
    // subdomain map
    const auto & increment_subdomain_map = remeshed.get_subdomain_name_map();
    main_subdomain_map.insert(increment_subdomain_map.begin(), increment_subdomain_map.end());

    full_mesh->stitch_meshes(remeshed,
                             primary_bcid,
                             paired_bcid,
                             TOLERANCE,
                             /*clear_stitched_bcids*/ getParam<bool>("clear_stitching_boundaries"),
                             verbose_stitching,
                             use_binary_search,
                             /*enforce_all_nodes_match_on_boundaries*/ false,
                             /*merge_boundary_nodes_all_or_nothing*/ false,
                             /*remap_subdomain_ids*/ !getParam<bool>("avoid_merging_subdomains"));

    // If we want to keep track of the stitching boundaries
    paired_bcid++;
    // TODO: when implementing hole meshes, we might want to also stitch the hole meshes
  }

  // We do not need the 3D mesh anymore
  mesh_3d->clear();
  // The stitching boundary should be removed by the stitcher, but this does not hurt
  if (getParam<bool>("clear_stitching_boundaries"))
    for (const auto bcid : make_range(starting_stitching_id, paired_bcid))
      full_mesh->get_boundary_info().remove_id(bcid);
  // Mesh is not prepared after stitching
  full_mesh->unset_is_prepared();

  return full_mesh;
}

Point
SurfaceSubdomainsDelaunayRemesher::elemNormal(const Elem & elem)
{
  mooseAssert(elem.n_vertices() == 3 || elem.n_vertices() == 4, "unsupported element type.");
  // Only the first three vertices are used to calculate the normal vector
  const Point & p0 = *elem.node_ptr(0);
  const Point & p1 = *elem.node_ptr(1);
  const Point & p2 = *elem.node_ptr(2);

  if (elem.n_vertices() == 4)
  {
    const Point & p3 = *elem.node_ptr(3);
    return ((p2 - p0).cross(p3 - p1)).unit();
  }

  return ((p2 - p1).cross(p0 - p1)).unit();
}

Point
SurfaceSubdomainsDelaunayRemesher::meshNormal2D(const MeshBase & mesh)
{
  Point mesh_norm = Point(0.0, 0.0, 0.0);
  Real mesh_area = 0.0;

  // Check all the elements' normal vectors
  for (const auto & elem : mesh.active_local_element_ptr_range())
  {
    const Real elem_area = elem->volume();
    mesh_norm += elemNormal(*elem) * elem_area;
    mesh_area += elem_area;
  }
  mesh.comm().sum(mesh_norm);
  mesh.comm().sum(mesh_area);
  mesh_norm /= mesh_area;
  return mesh_norm.unit();
}

Real
SurfaceSubdomainsDelaunayRemesher::meshNormalDeviation2D(const MeshBase & mesh,
                                                         const Point & global_norm)
{
  Real max_deviation(0.0);
  // Check all the elements' deviation from the global normal vector
  for (const auto & elem : mesh.active_local_element_ptr_range())
  {
    const Real elem_deviation = std::acos(global_norm * elemNormal(*elem)) / M_PI * 180.0;
    max_deviation = std::max(max_deviation, elem_deviation);
    if (_verbose && elem_deviation > _max_angle_deviation)
      _console << "Element " << elem->id() << " from subdomain ID " << elem->subdomain_id()
               << " has normal deviation: " << elem_deviation << std::endl;
  }
  mesh.comm().max(max_deviation);
  return max_deviation;
}

std::unique_ptr<ReplicatedMesh>
SurfaceSubdomainsDelaunayRemesher::General2DDelaunay(
    std::unique_ptr<ReplicatedMesh> & mesh_2d,
    std::vector<std::unique_ptr<ReplicatedMesh>> & hole_meshes_2d,
    unsigned int group_i)
{
  if (_verbose)
  {
    _console << "Re-meshing mesh\n " << *mesh_2d << std::endl;
    _console << "with subdomains " << Moose::stringify(mesh_2d->get_subdomain_name_map())
             << std::endl;
    if (hole_meshes_2d.size())
      _console << "With " << hole_meshes_2d.size() << " holes:" << std::endl;
    for (const auto & hole_m : hole_meshes_2d)
    {
      _console << "Hole subdomains " << Moose::stringify(hole_m->get_subdomain_name_map())
               << std::endl;
      _console << *hole_m << std::endl;
    }
  }
  // If a level set is provided, we need to check if the nodes in the original 2D mesh match the
  // level set
  if (_func_level_set)
  {
    for (const auto & node : mesh_2d->node_ptr_range())
    {
      if (std::abs(levelSetEvaluator(*node)) > libMesh::TOLERANCE)
      {
        paramError("level_set",
                   "The level set function does not match the nodes in the given boundary of the "
                   "input mesh. Level set evaluates at: " +
                       std::to_string(levelSetEvaluator(*node)) + " at node: " + node->get_info());
      }
    }
  }

  // Create the external boundary of the 2D mesh
  // Easier to work with a TRI3 mesh
  // all_tri() also prepares the mesh for use
  mesh_2d->prepare_for_use();
  bool has_external_side = false;
  MeshTools::Modification::all_tri(*mesh_2d);
  const auto mesh_2d_ext_bdry = MooseMeshUtils::getNextFreeBoundaryID(*mesh_2d);
  for (const auto elem : mesh_2d->active_element_ptr_range())
    for (const auto i_side : elem->side_index_range())
      if (elem->neighbor_ptr(i_side) == nullptr)
      {
        mesh_2d->get_boundary_info().add_side(elem, i_side, mesh_2d_ext_bdry);
        has_external_side = true;
      }

  // Create a clone of the 2D mesh to be used for the 1D mesh generation
  auto mesh_2d_dummy = dynamic_pointer_cast<MeshBase>(mesh_2d->clone());
  // Generate a new 1D block based on the external boundary
  const auto new_block_id_1d = MooseMeshUtils::getNextFreeSubdomainID(*mesh_2d_dummy);

  if (has_external_side)
    MooseMeshUtils::createSubdomainFromSidesets(*mesh_2d_dummy,
                                                {std::to_string(mesh_2d_ext_bdry)},
                                                new_block_id_1d,
                                                SubdomainName(),
                                                type());

  // Create a 1D mesh from the 1D block
  auto mesh_1d = buildMeshBaseObject();
  if (has_external_side)
    MooseMeshUtils::convertBlockToMesh(*mesh_2d_dummy, *mesh_1d, {std::to_string(new_block_id_1d)});
  mesh_2d_dummy->clear();

  // Add more nodes in the 1D mesh
  // We need to do this BEFORE the mesh is projected to avoid distortion in the node distances
  // The distortion is not the same for each subdomain, so we would end up with a non-conformal mesh
  if (isParamValid("max_edge_length"))
  {
    // We need to extract the points in the order of the boundary. This utility will build the
    // boundary as a loop
    // Add the nodeset from the sideset
    // NOTE: this is redoing the 1D mesh from the boundary
    mesh_2d->get_boundary_info().build_node_list_from_side_list({mesh_2d_ext_bdry});
    const auto boundary_1d = MooseMeshUtils::buildLoopBoundaryOf2DMesh(*mesh_2d, mesh_2d_ext_bdry);
    // Extract the points
    std::vector<Point> mesh_1d_points;
    mesh_1d_points.reserve(boundary_1d->n_nodes());
    for (const auto & node : boundary_1d->node_ptr_range())
      mesh_1d_points.push_back(*node);

    // Re-create the 1D mesh with a polyline with a maximum edge length
    mesh_1d->clear();
    // we add a tiny offset to avoid roundoff differences
    MooseMeshUtils::buildPolyLineMesh(
        *mesh_1d, mesh_1d_points, true, "", "", getParam<Real>("max_edge_length") + 1e-8);
  }

  // Find centroid of the 2D mesh
  const Point centroid = MooseMeshUtils::meshCentroidCalculator(*mesh_2d);
  // calculate an average normal vector of the 2D mesh
  const Point mesh_norm = meshNormal2D(*mesh_2d);
  // Check the deviation of the mesh normal vector from the global average normal vector
  if (meshNormalDeviation2D(*mesh_2d, mesh_norm) > _max_angle_deviation)
    paramError("subdomain_names",
               "The normal vector of some elements in the 2D mesh deviates too much from the "
               "global average normal vector. The maximum deviation found / allowed is " +
                   std::to_string(meshNormalDeviation2D(*mesh_2d, mesh_norm)) + " / " +
                   std::to_string(_max_angle_deviation) +
                   ". Consider dividing the boundary into several parts to "
                   "reduce the angle deviation.");

  // Move both 2d and 1d meshes to the centroid of the 2D mesh
  MeshTools::Modification::translate(*mesh_1d, -centroid(0), -centroid(1), -centroid(2));
  MeshTools::Modification::translate(*mesh_2d, -centroid(0), -centroid(1), -centroid(2));

  // Calculate the Euler angles to rotate the meshes so that the 2D mesh is close to the XY plane
  // (i.e., the normal vector of the 2D mesh is aligned with the Z axis)
  const Real theta = std::acos(mesh_norm(2)) / M_PI * 180.0;
  const Real phi =
      (MooseUtils::absoluteFuzzyLessThan(mesh_norm(2), 1.0) ? std::atan2(mesh_norm(1), mesh_norm(0))
                                                            : 0.0) /
      M_PI * 180.0;
  MeshTools::Modification::rotate(*mesh_1d, 90.0 - phi, theta, 0.0);
  MeshTools::Modification::rotate(*mesh_2d, 90.0 - phi, theta, 0.0);

  // Clone the 2D mesh to be used for reverse projection later
  auto mesh_2d_xyz = dynamic_pointer_cast<MeshBase>(mesh_2d->clone());

  // Project the 2D mesh to the XY plane so that XYDelaunay can be used
  for (const auto & node : mesh_2d->node_ptr_range())
    (*node)(2) = 0;
  // Project the 1D mesh to the XY plane as well
  for (const auto & node : mesh_1d->node_ptr_range())
    (*node)(2) = 0;

  // Finally, triangulation
  std::unique_ptr<ReplicatedMesh> mesh = dynamic_pointer_cast<ReplicatedMesh>(std::move(mesh_1d));

  Poly2TriTriangulator poly2tri(*mesh);
  poly2tri.triangulation_type() = TriangulatorInterface::PSLG;

  poly2tri.set_interpolate_boundary_points(_interpolate_boundaries[group_i]);
  poly2tri.set_verify_hole_boundaries(false);
  poly2tri.desired_area() = _desired_areas[group_i];
  poly2tri.minimum_angle() = 0; // Not yet supported
  poly2tri.smooth_after_generating() = false;
  // Future TODO: correct the area function based on the local normal vector
  if (_use_auto_area_func)
    poly2tri.set_auto_area_function(this->comm(),
                                    _auto_area_function_num_points,
                                    _auto_area_function_power,
                                    _auto_area_func_default_size,
                                    _auto_area_func_default_size_dist);
  poly2tri.triangulate();

  // Reverse the projection based on the original 2D mesh
  for (const auto & node : mesh->node_ptr_range())
  {
    bool node_mod = false;
    // Try to find the element in mesh_2d that contains the new node
    for (const auto & elem : mesh_2d->active_element_ptr_range())
    {
      if (elem->contains_point(Point((*node)(0), (*node)(1), 0.0)))
      {
        // Element id
        const auto elem_id = elem->id();
        // element in xyz_in_xyz
        const Elem & elem_xyz = *mesh_2d_xyz->elem_ptr(elem_id);

        const Point elem_normal = elemNormal(elem_xyz);
        const Point & elem_p = *mesh_2d_xyz->elem_ptr(elem_id)->node_ptr(0);

        // if the x and y values of the node is the same as the elem_p's first node, we can just
        // move it to that node's position
        if (MooseUtils::absoluteFuzzyEqual((*node)(0), elem_p(0)) &&
            MooseUtils::absoluteFuzzyEqual((*node)(1), elem_p(1)))
        {
          (*node)(2) = elem_p(2);
          node_mod = true;
          break;
        }
        // Otherwise, we need to find a position inside the 2D element
        // It has the same x and y coordinates as the node in the projected mesh;
        (*node)(2) = elem_p(2) - (((*node)(0) - elem_p(0)) * elem_normal(0) +
                                  ((*node)(1) - elem_p(1)) * elem_normal(1)) /
                                     elem_normal(2);
        node_mod = true;
        break;
      }
    }
    if (!node_mod)
      mooseError("Node not found in mesh_in_xy");
  }

  // Rotate the mesh back
  MeshTools::Modification::rotate(*mesh, 0.0, -theta, phi - 90.0);
  // Translate the mesh back
  MeshTools::Modification::translate(*mesh, centroid(0), centroid(1), centroid(2));

  // Correct the nodes based on the level set function
  if (_func_level_set)
  {
    for (const auto & node : mesh->node_ptr_range())
    {
      unsigned int iter_ct = 0;
      while (iter_ct < _max_level_set_correction_iterations &&
             std::abs(levelSetEvaluator(*node)) > libMesh::TOLERANCE * libMesh::TOLERANCE)
      {
        levelSetCorrection(*node);
        ++iter_ct;
      }
    }
  }
  // Give the old subdomain to all elements
  const auto common_id = *mesh_2d->get_mesh_subdomains().begin();
  for (auto & elem : mesh->active_element_ptr_range())
    elem->subdomain_id() = common_id;

  // Pass the subdomain names
  mesh->set_subdomain_name_map() = mesh_2d->get_subdomain_name_map();
  // Remove the boundaries, they get re-added (with a known ID) when stitching the pieces together
  mesh->get_boundary_info().remove_id(mesh_2d_ext_bdry);
  // Elements have changed, neighbors, ids etc
  mesh->unset_is_prepared();

  return mesh;
}
