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
#include "libmesh/mesh_serializer.h"
#include "libmesh/parallel_algebra.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", SurfaceSubdomainsDelaunayRemesher);

InputParameters
SurfaceSubdomainsDelaunayRemesher::validParams()
{
  InputParameters params = SurfaceDelaunayGeneratorBase::validParams();
  params += FunctionParserUtils<false>::validParams();

  params.addClassDescription(
      "Mesh generator that re-meshes a 2D surface mesh given as one or more subdomains into "
      "a 2D surface mesh using Delaunay triangulation.");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addRequiredParam<std::vector<std::vector<SubdomainName>>>("subdomain_names", "The surface mesh subdomains to be re-mesheed");
  params.addParam<std::vector<std::vector<BoundaryName>>>(
      "hole_boundary_names",
      std::vector<std::vector<BoundaryName>>(),
      "The optional boundaries to be used as the holes in the mesh during triangulation. Note that "
      "this is a vector of vectors, which allows each hole to be defined as a combination of "
      "multiple boundaries.");

  // Note: if the Delaunay generator can handle random topology for the surface, mesh everything in
  // a single step. If not, then mesh each part one by one

  // Mesh density and shape parameters
  params.addParam<std::string>(
      "level_set",
      "Level set used to achieve more accurate reverse projection compared to interpolation.");
  params.addParam<unsigned int>(
      "max_level_set_correction_iterations",
      3,
      "Maximum number of iterations to correct the nodes based on the level set function.");
  params.addRangeCheckedParam<Real>(
      "max_angle_deviation",
      60.0,
      "max_angle_deviation>0 & max_angle_deviation<90",
      "Maximum angle deviation from the global average normal vector in the input mesh.");

  // When re-building
  params.addParam<bool>("avoid_merging_subdomains",
                        false,
                        "Whether to prevent merging subdomains by offsetting ids. The first mesh "
                        "in the input will keep the same subdomains ids, the others will have "
                        "offsets. All subdomain names will remain valid");
  params.addParam<bool>("avoid_merging_boundaries",
                        false,
                        "Whether to prevent merging sidesets by offsetting ids. The first mesh "
                        "in the input will keep the same boundary ids, the others will have "
                        "offsets. All boundary names will remain valid");

  params.addParam<bool>(
      "verbose", false, "Whether the generator should output additional information");

  return params;
}

SurfaceSubdomainsDelaunayRemesher::SurfaceSubdomainsDelaunayRemesher(const InputParameters & parameters)
  : SurfaceDelaunayGeneratorBase(parameters),
    FunctionParserUtils<false>(parameters),
    _input(getMesh("input")),
    _subdomain_names(getParam<std::vector<std::vector<SubdomainName>>>("subdomain_names")),
    _num_groups(_subdomain_names.size()),
    _hole_boundary_names(getParam<std::vector<std::vector<BoundaryName>>>("hole_boundary_names")),
    _max_level_set_correction_iterations(
        getParam<unsigned int>("max_level_set_correction_iterations")),
    _max_angle_deviation(getParam<Real>("max_angle_deviation")),
    _verbose(getParam<bool>("verbose"))
{
  if (isParamValid("level_set"))
  {
    _func_level_set = std::make_shared<SymFunction>();
    // set FParser internal feature flags
    setParserFeatureFlags(_func_level_set);
    if (isParamValid("constant_names") && isParamValid("constant_expressions"))
      addFParserConstants(_func_level_set,
                          getParam<std::vector<std::string>>("constant_names"),
                          getParam<std::vector<std::string>>("constant_expressions"));
    if (_func_level_set->Parse(getParam<std::string>("level_set"), "x,y,z") >= 0)
      mooseError("Invalid function f(x,y,z)\n",
                 _func_level_set,
                 "\nin ",
                 name(),
                 ".\n",
                 _func_level_set->ErrorMsg());

    _func_params.resize(3);
  }
}

std::unique_ptr<MeshBase>
SurfaceSubdomainsDelaunayRemesher::generate()
{
  std::unique_ptr<MeshBase> mesh_3d = std::move(_input);

  // If holes are provided, we need to create new blocks for them too
  std::vector<subdomain_id_type> hole_block_ids;
  // for (const auto & hole : _hole_boundary_names)
  // {
  //   hole_block_ids.push_back(MooseMeshUtils::getNextFreeSubdomainID(*mesh_3d));
  //   try
  //   {
  //     MooseMeshUtils::createSubdomainFromSidesets(
  //         *mesh_3d, hole, hole_block_ids.back(), SubdomainName(), type());
  //   }
  //   catch (MooseException & e)
  //   {
  //     if (((std::string)e.what()).compare(0, 12, "The sideset ") == 0)
  //       paramError("hole_boundary_names", e.what());
  //     else
  //       mooseError(e.what());
  //   }
  // }

  // Re-create surface meshes from each of the subdomains
  std::unique_ptr<ReplicatedMesh> full_mesh;
  std::vector<std::unique_ptr<ReplicatedMesh>> hole_meshes_2d;

  for (const auto i : make_range(_num_groups))
  {
    auto mesh_2d = buildReplicatedMesh();
    MooseMeshUtils::convertBlockToMesh(*mesh_3d, *mesh_2d, _subdomain_names[i]);

    // If holes are provided, we need to create a 2D mesh for each hole
    for (const auto & hole_block_id : hole_block_ids)
    {
      hole_meshes_2d.push_back(buildReplicatedMesh());
      MooseMeshUtils::convertBlockToMesh(
          *mesh_3d, *hole_meshes_2d.back(), {std::to_string(hole_block_id)});
    }

    // Mesh the subdomains by groupsp
    auto new_mesh = General2DDelaunay(mesh_2d, hole_meshes_2d);

    // Add the newly meshed region to the holes
    hole_meshes_2d.push_back(std::move(new_mesh));
  }

  // These should become parameters
  const auto inner_bcid = 1;
  const auto new_hole_bcid = 1;
  const auto _verbose_stitching = true;
  const auto use_binary_search = true;
  std::vector<bool> _stitch_holes(hole_meshes_2d.size(), true);

  // Stitch all the parts
  // Define a reference map variable for subdomain map
  full_mesh = std::move(hole_meshes_2d[0]);
  auto & main_subdomain_map = full_mesh->set_subdomain_name_map();
  for (auto hole_i : index_range(hole_meshes_2d))
  {
    if (hole_i == 0)
      continue;
    if (hole_i < _stitch_holes.size() && _stitch_holes[hole_i])
    {
      UnstructuredMesh & hole_mesh = dynamic_cast<UnstructuredMesh &>(*hole_meshes_2d[hole_i]);
      // increase hole mesh order if the triangulation mesh has higher order
      // if (!holes_with_midpoints[hole_i])
      // {
      //   if (_tri_elem_type == "TRI6")
      //     hole_mesh.all_second_order();
      //   else if (_tri_elem_type == "TRI7")
      //     hole_mesh.all_complete_order();
      // }
      auto & hole_boundary_info = hole_mesh.get_boundary_info();

      // Our algorithm here requires a serialized Mesh.  To avoid
      // redundant serialization and deserialization (libMesh
      // MeshedHole and stitch_meshes still also require
      // serialization) we'll do the serialization up front.
      libMesh::MeshSerializer serial_hole(hole_mesh);

      // It would have been nicer for MeshedHole to add the BCID
      // itself, but we want MeshedHole to work with a const mesh.
      // We'll still use MeshedHole, for its code distinguishing
      // outer boundaries from inner boundaries on a
      // hole-with-holes.
      libMesh::TriangulatorInterface::MeshedHole mh{hole_mesh};

      // We have to translate from MeshedHole points to mesh
      // sides.
      std::unordered_map<Point, Point> next_hole_boundary_point;
      const int np = mh.n_points();
      for (auto pi : make_range(1, np))
        next_hole_boundary_point[mh.point(pi - 1)] = mh.point(pi);
      next_hole_boundary_point[mh.point(np - 1)] = mh.point(0);

#ifndef NDEBUG
      int found_hole_sides = 0;
#endif
      for (auto elem : hole_mesh.element_ptr_range())
      {
        if (elem->dim() != 2)
          mooseError("Non 2-D element found in hole; stitching is not supported.");

        auto ns = elem->n_sides();
        for (auto s : make_range(ns))
        {
          auto it_s = next_hole_boundary_point.find(elem->point(s));
          if (it_s != next_hole_boundary_point.end())
            if (it_s->second == elem->point((s + 1) % ns))
            {
              hole_boundary_info.add_side(elem, s, new_hole_bcid);
#ifndef NDEBUG
              ++found_hole_sides;
#endif
            }
        }
      }
      if (_verbose)
        _console << "Found " << found_hole_sides << " connecting sides on hole #" << hole_i << std::endl;
      // mooseAssert(found_hole_sides == np, "Failed to find full outer boundary of meshed hole");

      auto & mesh_boundary_info = full_mesh->get_boundary_info();
#ifndef NDEBUG
      // int found_inner_sides = 0;
#endif
      for (auto elem : full_mesh->element_ptr_range())
      {
        auto ns = elem->n_sides();
        for (auto s : make_range(ns))
        {
          auto it_s = next_hole_boundary_point.find(elem->point((s + 1) % ns));
          if (it_s != next_hole_boundary_point.end())
            if (it_s->second == elem->point(s))
            {
              mesh_boundary_info.add_side(elem, s, inner_bcid);
#ifndef NDEBUG
              // ++found_inner_sides;
#endif
            }
        }
      }
      // mooseAssert(found_inner_sides == np, "Failed to find full boundary around meshed hole");

      // Retrieve subdomain name map from the mesh to be stitched and insert it into the main
      // subdomain map
      const auto & increment_subdomain_map = hole_mesh.get_subdomain_name_map();
      std::cout << Moose::stringify(increment_subdomain_map) << std::endl;

      main_subdomain_map.insert(increment_subdomain_map.begin(), increment_subdomain_map.end());
      std::cout << Moose::stringify(main_subdomain_map) << std::endl;

      full_mesh->stitch_meshes(hole_mesh,
                               inner_bcid,
                               new_hole_bcid,
                               TOLERANCE,
                               /*clear_stitched_bcids*/ true,
                               _verbose_stitching,
                               use_binary_search,
                               /*enforce_all_nodes_match_on_boundaries*/ false,
                               /*merge_boundary_nodes_all_or_nothing*/ false,
                               /*remap_subdomain_ids*/ true);
    }
  }

  // We do not need the 3D mesh anymore
  mesh_3d->clear();

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
  }
  mesh.comm().max(max_deviation);
  return max_deviation;
}

Real
SurfaceSubdomainsDelaunayRemesher::levelSetEvaluator(const Point & point)
{
  return evaluate(_func_level_set, std::vector<Real>({point(0), point(1), point(2), 0}));
}

void
SurfaceSubdomainsDelaunayRemesher::levelSetCorrection(Node & node)
{
  // Based on the given level set, we try to move the node in its normal direction
  const Real diff = libMesh::TOLERANCE * 10.0; // A small value to perturb the node
  const Real original_eval = levelSetEvaluator(node);
  const Real xp_eval = levelSetEvaluator(node + Point(diff, 0.0, 0.0));
  const Real yp_eval = levelSetEvaluator(node + Point(0.0, diff, 0.0));
  const Real zp_eval = levelSetEvaluator(node + Point(0.0, 0.0, diff));
  const Real xm_eval = levelSetEvaluator(node - Point(diff, 0.0, 0.0));
  const Real ym_eval = levelSetEvaluator(node - Point(0.0, diff, 0.0));
  const Real zm_eval = levelSetEvaluator(node - Point(0.0, 0.0, diff));
  const Point grad = Point((xp_eval - xm_eval) / (2.0 * diff),
                           (yp_eval - ym_eval) / (2.0 * diff),
                           (zp_eval - zm_eval) / (2.0 * diff));
  const Real xyz_diff = -original_eval / grad.contract(grad);
  node(0) += xyz_diff * grad(0);
  node(1) += xyz_diff * grad(1);
  node(2) += xyz_diff * grad(2);
}

std::unique_ptr<ReplicatedMesh>
SurfaceSubdomainsDelaunayRemesher::General2DDelaunay(
    std::unique_ptr<ReplicatedMesh> & mesh_2d,
    std::vector<std::unique_ptr<ReplicatedMesh>> & hole_meshes_2d)
{
  if (_verbose)
  {
    _console << "Re-meshing mesh\n " << *mesh_2d << std::endl;
    if (hole_meshes_2d.size())
      _console << "With " << hole_meshes_2d.size() << " holes:" << std::endl;
    for (const auto & hole_m : hole_meshes_2d)
      _console << *hole_m << std::endl;
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
                   "input mesh.");
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
  std::cout << "new bdy " << mesh_2d_ext_bdry << std::endl;
  for (const auto & elem : mesh_2d->active_element_ptr_range())
    for (const auto & i_side : elem->side_index_range())
      if (elem->neighbor_ptr(i_side) == nullptr)
      {
        mesh_2d->get_boundary_info().add_side(elem, i_side, mesh_2d_ext_bdry);
        has_external_side = true;
      }
  std::cout << "Found an external side " << has_external_side << std::endl;

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

  // If we have holes, we need to create a 1D mesh for each hole
  std::vector<std::unique_ptr<MeshBase>> hole_meshes_1d;
  for (auto & hole_mesh_2d : hole_meshes_2d)
  {
      // As we do not need these holes for reverse projection, we do not need to convert them to TRI3
      // meshes, but we still need to create a 1D mesh for each hole
      hole_mesh_2d->find_neighbors();
      bool has_external_bdy = false;
      const auto hole_mesh_2d_ext_bdry = MooseMeshUtils::getNextFreeBoundaryID(*hole_mesh_2d);
      MooseMeshUtils::addExternalBoundary(*hole_mesh_2d,
                                          hole_mesh_2d_ext_bdry,
                                          has_external_bdy);

      // Only remesh if they are not already meshed
      if (false)
      {
        if (_verbose)
          std::cout << "Remeshing hole " << *hole_mesh_2d << std::endl;

        const auto new_hole_block_id_1d = MooseMeshUtils::getNextFreeSubdomainID(*hole_mesh_2d);
        std::cout << has_external_bdy << " bdy id " << hole_mesh_2d_ext_bdry << " new block id " << new_hole_block_id_1d << std::endl;
        if (has_external_bdy)
          MooseMeshUtils::createSubdomainFromSidesets(*hole_mesh_2d,
                                                      {std::to_string(hole_mesh_2d_ext_bdry)},
                                                      new_hole_block_id_1d,
                                                      SubdomainName(),
                                                      type());
        // Create a 1D mesh form the 1D block
        hole_meshes_1d.push_back(buildMeshBaseObject());
        if (has_external_bdy)
          MooseMeshUtils::convertBlockToMesh(
              *hole_mesh_2d, *hole_meshes_1d.back(), {std::to_string(new_hole_block_id_1d)});
      }
      else
        // Extract the 1D edge elements boundary as a mesh
        hole_meshes_1d.push_back(MooseMeshUtils::buildBoundaryMesh(*hole_mesh_2d, hole_mesh_2d_ext_bdry));

    // Do not clear the 2D hole mesh, we may re-use it
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
  // Also need to translate the 1D hole meshes if applicable
  for (auto & hole_mesh_1d : hole_meshes_1d)
    MeshTools::Modification::translate(*hole_mesh_1d, -centroid(0), -centroid(1), -centroid(2));

  // Calculate the Euler angles to rotate the meshes so that the 2D mesh is close to the XY plane
  // (i.e., the normal vector of the 2D mesh is aligned with the Z axis)
  const Real theta = std::acos(mesh_norm(2)) / M_PI * 180.0;
  const Real phi =
      (MooseUtils::absoluteFuzzyLessThan(mesh_norm(2), 1.0) ? std::atan2(mesh_norm(1), mesh_norm(0))
                                                            : 0.0) /
      M_PI * 180.0;
  MeshTools::Modification::rotate(*mesh_1d, 90.0 - phi, theta, 0.0);
  MeshTools::Modification::rotate(*mesh_2d, 90.0 - phi, theta, 0.0);
  // Also rotate the 1D hole meshes if applicable
  for (auto & hole_mesh_1d : hole_meshes_1d)
    MeshTools::Modification::rotate(*hole_mesh_1d, 90.0 - phi, theta, 0.0);

  // Clone the 2D mesh to be used for reverse projection later
  auto mesh_2d_xyz = dynamic_pointer_cast<MeshBase>(mesh_2d->clone());

  // Project the 2D mesh to the XY plane so that XYDelaunay can be used
  for (const auto & node : mesh_2d->node_ptr_range())
    (*node)(2) = 0;
  // Project the 1D mesh to the XY plane as well
  for (const auto & node : mesh_1d->node_ptr_range())
    (*node)(2) = 0;
  // Also project the 1D hole meshes to the XY plane if applicable
  for (auto & hole_mesh_1d : hole_meshes_1d)
  {
    for (const auto & node : hole_mesh_1d->node_ptr_range())
      (*node)(2) = 0;
  }

  std::vector<libMesh::TriangulatorInterface::MeshedHole> meshed_holes;
  std::vector<libMesh::TriangulatorInterface::Hole *> triangulator_hole_ptrs;
  meshed_holes.reserve(hole_meshes_1d.size());
  triangulator_hole_ptrs.reserve(hole_meshes_1d.size());
  for (auto hole_i : index_range(hole_meshes_1d))
  {
    hole_meshes_1d[hole_i]->prepare_for_use();
    meshed_holes.emplace_back(*hole_meshes_1d[hole_i]);
    // if (hole_connected[hole_i])
      triangulator_hole_ptrs[hole_i] = &meshed_holes.back();
  }

  // Finally, triangulation
  std::unique_ptr<ReplicatedMesh> mesh = dynamic_pointer_cast<ReplicatedMesh>(std::move(mesh_1d));

  Poly2TriTriangulator poly2tri(*mesh);
  poly2tri.triangulation_type() = TriangulatorInterface::PSLG;

  poly2tri.set_interpolate_boundary_points(0);
  poly2tri.set_refine_boundary_allowed(false);
  poly2tri.set_verify_hole_boundaries(false);
  poly2tri.desired_area() = 0;
  poly2tri.minimum_angle() = 0; // Not yet supported
  poly2tri.smooth_after_generating() = false;
  if (!triangulator_hole_ptrs.empty())
    poly2tri.attach_hole_list(&triangulator_hole_ptrs);
  // Future TODO: correct the area function based on the local normal vector
  if (_use_auto_area_func)
    poly2tri.set_auto_area_function(this->comm(),
                                    _auto_area_function_num_points,
                                    _auto_area_function_power,
                                    _auto_area_func_default_size,
                                    _auto_area_func_default_size_dist);
  poly2tri.triangulate();
  // std::cout << "Triangulated and got " << *mesh << std::endl;

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

  mesh->set_isnt_prepared();

  return mesh;
}
