//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshTriangulationUtils.h"
#include "MooseMeshUtils.h"
#include "CastUniquePointer.h"

#include "libmesh/elem.h"
#include "libmesh/boundary_info.h"
#include "libmesh/int_range.h"
#include "libmesh/enum_to_string.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/parsed_function.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/unstructured_mesh.h"

using namespace libMesh;

namespace MeshTriangulationUtils
{

std::unique_ptr<MeshBase>
triangulateWithDelaunay(MeshGenerator & mg,
                        std::unique_ptr<MeshBase> boundary_mesh,
                        std::vector<std::unique_ptr<MeshBase>> hole_meshes,
                        const XYDelaunayOptions & xyd_opts)
{
  // Put the boundary mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(boundary_mesh));

  // Get ready to triangulate the line segments we extract from it
  libMesh::Poly2TriTriangulator poly2tri(*mesh);
  poly2tri.triangulation_type() = libMesh::TriangulatorInterface::PSLG;

  // If we're using a user-requested subset of boundaries on that
  // mesh, get their ids.
  std::set<std::size_t> bdy_ids;

  if (!xyd_opts.input_boundary_names.empty())
  {
    if (!xyd_opts.input_subdomain_names.empty())
      mg.paramError(
          "input_subdomain_names",
          "input_boundary_names and input_subdomain_names cannot both specify an outer boundary.");

    for (const auto & name : xyd_opts.input_boundary_names)
    {
      auto bcid = MooseMeshUtils::getBoundaryID(name, *mesh);
      if (bcid == BoundaryInfo::invalid_id)
        mg.paramError("input_boundary_names", name, " is not a boundary name in the input mesh");

      bdy_ids.insert(bcid);
    }
  }

  if (!xyd_opts.input_subdomain_names.empty())
  {
    // Make sure subdomain info caches are up to date
    if (!mesh->preparation().has_cached_elem_data)
      mesh->cache_elem_data();

    const auto subdomain_ids =
        MooseMeshUtils::getSubdomainIDs(*mesh, xyd_opts.input_subdomain_names);

    // Check that the requested subdomains exist in the mesh
    std::set<SubdomainID> subdomains;
    mesh->subdomain_ids(subdomains);

    for (auto i : index_range(subdomain_ids))
    {
      if (subdomain_ids[i] == Moose::INVALID_BLOCK_ID || !subdomains.count(subdomain_ids[i]))
        mg.paramError("input_subdomain_names",
                      xyd_opts.input_subdomain_names[i],
                      " was not found in the boundary mesh");

      bdy_ids.insert(subdomain_ids[i]);
    }
  }

  if (!bdy_ids.empty())
    poly2tri.set_outer_boundary_ids(bdy_ids);

  poly2tri.set_interpolate_boundary_points(xyd_opts.add_nodes_per_boundary_segment);
  poly2tri.set_refine_boundary_allowed(xyd_opts.refine_bdy);
  poly2tri.set_verify_hole_boundaries(xyd_opts.verify_holes);

  poly2tri.desired_area() = xyd_opts.desired_area;
  poly2tri.minimum_angle() = 0; // Not yet supported
  poly2tri.smooth_after_generating() = xyd_opts.smooth_tri;

  std::vector<libMesh::TriangulatorInterface::MeshedHole> meshed_holes;
  std::vector<libMesh::TriangulatorInterface::Hole *> triangulator_hole_ptrs(hole_meshes.size());
  // This tells us the element orders of the hole meshes
  // For the boundary meshes, it can be access through poly2tri.segment_midpoints.
  std::vector<bool> holes_with_midpoints(hole_meshes.size());
  bool stitch_second_order_holes(false);

  // Make sure pointers here aren't invalidated by a resize
  meshed_holes.reserve(hole_meshes.size());
  for (auto hole_i : index_range(hole_meshes))
  {
    if (!hole_meshes[hole_i]->is_prepared())
      hole_meshes[hole_i]->prepare_for_use();
    if (hole_i < xyd_opts.hole_boundary_id_filters.size() &&
        !xyd_opts.hole_boundary_id_filters[hole_i].empty())
      meshed_holes.emplace_back(*hole_meshes[hole_i], xyd_opts.hole_boundary_id_filters[hole_i]);
    else
      meshed_holes.emplace_back(*hole_meshes[hole_i]);
    holes_with_midpoints[hole_i] = meshed_holes.back().n_midpoints();
    stitch_second_order_holes =
        xyd_opts.stitch_holes.empty()
            ? false
            : ((holes_with_midpoints[hole_i] && xyd_opts.stitch_holes[hole_i]) ||
               stitch_second_order_holes);
    if (hole_i < xyd_opts.refine_holes.size())
      meshed_holes.back().set_refine_boundary_allowed(xyd_opts.refine_holes[hole_i]);

    triangulator_hole_ptrs[hole_i] = &meshed_holes.back();
  }
  if (stitch_second_order_holes &&
      (xyd_opts.tri_elem_type == "TRI3" || xyd_opts.tri_elem_type == "DEFAULT"))
    mg.paramError(
        "tri_element_type",
        "Cannot use first order elements with stitched quadratic element holes. Please try "
        "to specify a higher-order tri_element_type or reduce the order of the hole inputs.");

  if (!triangulator_hole_ptrs.empty())
    poly2tri.attach_hole_list(&triangulator_hole_ptrs);

  if (xyd_opts.desired_area_func != "")
  {
    // poly2tri will clone this so it's fine going out of scope
    libMesh::ParsedFunction<Real> area_func{xyd_opts.desired_area_func};
    poly2tri.set_desired_area_function(&area_func);
  }
  else if (xyd_opts.use_auto_area_func)
  {
    poly2tri.set_auto_area_function(
        mg.comm(),
        xyd_opts.auto_area_function_num_points,
        xyd_opts.auto_area_function_power,
        xyd_opts.auto_area_func_default_size > 0.0 ? xyd_opts.auto_area_func_default_size : 0.0,
        xyd_opts.auto_area_func_default_size_dist > 0.0 ? xyd_opts.auto_area_func_default_size_dist
                                                        : -1.0);
  }

  if (xyd_opts.tri_elem_type == "TRI6")
    poly2tri.elem_type() = libMesh::ElemType::TRI6;
  else if (xyd_opts.tri_elem_type == "TRI7")
    poly2tri.elem_type() = libMesh::ElemType::TRI7;
  // Add interior points before triangulating. Only points inside the boundaries
  // will be meshed.
  for (const auto & point : xyd_opts.interior_points)
    mesh->add_point(point);

  poly2tri.triangulate();

  SubdomainID output_subdomain_id =
      xyd_opts.has_output_subdomain_id ? xyd_opts.output_subdomain_id : 0;

  if (xyd_opts.has_output_subdomain_name)
  {
    auto id = MooseMeshUtils::getSubdomainID(xyd_opts.output_subdomain_name, *mesh);

    if (id == Elem::invalid_subdomain_id)
    {
      if (!xyd_opts.has_output_subdomain_id)
      {
        // We'll probably need to make a new ID, then
        output_subdomain_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);

        // But check the hole meshes for our output subdomain name too
        for (auto & hole_ptr : hole_meshes)
        {
          auto possible_sbdid =
              MooseMeshUtils::getSubdomainID(xyd_opts.output_subdomain_name, *hole_ptr);
          // Huh, it was in one of them
          if (possible_sbdid != Elem::invalid_subdomain_id)
          {
            output_subdomain_id = possible_sbdid;
            break;
          }
          output_subdomain_id =
              std::max(output_subdomain_id, MooseMeshUtils::getNextFreeSubdomainID(*hole_ptr));
        }
      }
    }
    else
    {
      if (xyd_opts.has_output_subdomain_id)
      {
        if (id != output_subdomain_id)
          mg.paramError("output_subdomain_name",
                        "name has been used by the input meshes and the corresponding id is not "
                        "equal to 'output_subdomain_id'");
      }
      else
        output_subdomain_id = id;
    }
    // We do not want to set an empty subdomain name
    if (xyd_opts.output_subdomain_name.size())
      mesh->subdomain_name(output_subdomain_id) = xyd_opts.output_subdomain_name;
  }

  if (xyd_opts.smooth_tri || output_subdomain_id)
    for (auto elem : mesh->element_ptr_range())
    {
      mooseAssert(elem->type() == (xyd_opts.tri_elem_type == "TRI6"
                                       ? TRI6
                                       : (xyd_opts.tri_elem_type == "TRI7" ? TRI7 : TRI3)),
                  "Unexpected element type " << libMesh::Utility::enum_to_string(elem->type())
                                             << " found in triangulation");

      elem->subdomain_id() = output_subdomain_id;

      // I do not trust Laplacian mesh smoothing not to invert
      // elements near reentrant corners.  Eventually we'll add better
      // smoothing options, but even those might have failure cases.
      // Better to always do extra tests here than to ever let users
      // try to run on a degenerate mesh.
      if (xyd_opts.smooth_tri)
      {
        auto cross_prod = (elem->point(1) - elem->point(0)).cross(elem->point(2) - elem->point(0));

        if (cross_prod(2) <= 0)
          mooseError("Inverted element found in triangulation.\n"
                     "Laplacian smoothing can create these at reentrant corners; disable it?");
      }
    }

  // The hole meshes are specified by the user, so they could have any
  // BCID or no BCID or any combination of BCIDs on their outer
  // boundary, so we'll have to set our own BCID to use for stitching
  // there.  We'll need to check all the holes for used BCIDs, if we
  // want to pick a new ID on hole N that doesn't conflict with any
  // IDs on hole M < N (or with the IDs on the new triangulation)

  // The new triangulation by default assigns BCID i+1 to hole i ...
  // but we can't even use this for mesh stitching, because we can't
  // be sure it isn't also already in use on the hole's mesh and so we
  // won't be able to safely clear it afterwards.
  const boundary_id_type end_bcid = hole_meshes.size() + 1;

  // If a hole has its boundary layer mesh, we need to move the hole bcid to the "real" hole
  // boundary in the boundary layer mesh. So we need to record them here.
  std::vector<BoundaryID> hole_boundary_rec(hole_meshes.size());
  std::iota(hole_boundary_rec.begin(), hole_boundary_rec.end(), 1);

  // For the hole meshes that need to be stitched, we would like to make sure the hole boundary ids
  // and output boundary id are not conflicting with the existing boundary ids of the hole meshes to
  // be stitched.
  BoundaryID free_boundary_id = 0;
  if (xyd_opts.stitch_holes.size())
  {
    for (auto hole_i : index_range(hole_meshes))
    {
      if (xyd_opts.stitch_holes[hole_i])
      {
        free_boundary_id =
            std::max(free_boundary_id, MooseMeshUtils::getNextFreeBoundaryID(*hole_meshes[hole_i]));
        hole_meshes[hole_i]->comm().max(free_boundary_id);
      }
    }
    for (auto h : index_range(hole_meshes))
    {
      libMesh::MeshTools::Modification::change_boundary_id(*mesh, h + 1, h + 1 + free_boundary_id);
      hole_boundary_rec[h] = h + 1 + free_boundary_id;
    }
  }
  boundary_id_type new_hole_bcid = end_bcid + free_boundary_id;

  // We might be overriding the default bcid numbers.  We have to be
  // careful about how we renumber, though.  We pick unused temporary
  // numbers because e.g. "0->2, 2->0" is impossible to do
  // sequentially, but "0->N, 2->N+2, N->2, N+2->0" works.
  libMesh::MeshTools::Modification::change_boundary_id(
      *mesh, 0, (xyd_opts.has_output_boundary ? end_bcid : 0) + free_boundary_id);

  if (!xyd_opts.hole_boundaries.empty())
  {
    auto hole_boundary_ids = MooseMeshUtils::getBoundaryIDs(*mesh, xyd_opts.hole_boundaries, true);

    for (auto h : index_range(hole_meshes))
      libMesh::MeshTools::Modification::change_boundary_id(
          *mesh, h + 1 + free_boundary_id, h + 1 + free_boundary_id + end_bcid);

    for (auto h : index_range(hole_meshes))
    {
      libMesh::MeshTools::Modification::change_boundary_id(
          *mesh, h + 1 + free_boundary_id + end_bcid, hole_boundary_ids[h]);
      hole_boundary_rec[h] = hole_boundary_ids[h];
      mesh->get_boundary_info().sideset_name(hole_boundary_ids[h]) = xyd_opts.hole_boundaries[h];
      new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(hole_boundary_ids[h] + 1));
    }
  }

  if (xyd_opts.has_output_boundary)
  {
    const std::vector<BoundaryID> output_boundary_id =
        MooseMeshUtils::getBoundaryIDs(*mesh, {xyd_opts.output_boundary}, true);

    libMesh::MeshTools::Modification::change_boundary_id(
        *mesh, end_bcid + free_boundary_id, output_boundary_id[0]);
    mesh->get_boundary_info().sideset_name(output_boundary_id[0]) = xyd_opts.output_boundary;

    new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(output_boundary_id[0] + 1));
  }

  bool doing_stitching = false;

  for (auto hole_i : index_range(hole_meshes))
  {
    const MeshBase & hole_mesh = *hole_meshes[hole_i];
    auto & hole_boundary_info = hole_mesh.get_boundary_info();
    const std::set<boundary_id_type> & local_hole_bcids = hole_boundary_info.get_boundary_ids();

    if (!local_hole_bcids.empty())
      new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(*local_hole_bcids.rbegin() + 1));
    hole_mesh.comm().max(new_hole_bcid);

    if (hole_i < xyd_opts.stitch_holes.size() && xyd_opts.stitch_holes[hole_i])
      doing_stitching = true;
  }

  const boundary_id_type inner_bcid = new_hole_bcid + 1;

  // libMesh mesh stitching still requires a serialized mesh, and it's
  // cheaper to do that once than to do it once-per-hole
  libMesh::MeshSerializer serial(*mesh, doing_stitching);

  // Define a reference map variable for subdomain map
  auto & main_subdomain_map = mesh->set_subdomain_name_map();
  for (auto hole_i : index_range(hole_meshes))
  {
    if (hole_i < xyd_opts.stitch_holes.size() && xyd_opts.stitch_holes[hole_i])
    {
      UnstructuredMesh & hole_mesh = dynamic_cast<UnstructuredMesh &>(*hole_meshes[hole_i]);
      // increase hole mesh order if the triangulation mesh has higher order
      if (!holes_with_midpoints[hole_i])
      {
        if (xyd_opts.tri_elem_type == "TRI6")
          hole_mesh.all_second_order();
        else if (xyd_opts.tri_elem_type == "TRI7")
          hole_mesh.all_complete_order();
      }
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
      const auto & hole_bdy_id_filter = (hole_i < xyd_opts.hole_boundary_id_filters.size())
                                            ? xyd_opts.hole_boundary_id_filters[hole_i]
                                            : std::set<std::size_t>();
      libMesh::TriangulatorInterface::MeshedHole mh{hole_mesh, hole_bdy_id_filter};

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
      mooseAssert(found_hole_sides == np, "Failed to find full outer boundary of meshed hole");

      auto & mesh_boundary_info = mesh->get_boundary_info();
#ifndef NDEBUG
      int found_inner_sides = 0;
#endif
      for (auto elem : mesh->element_ptr_range())
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
              ++found_inner_sides;
#endif
            }
        }
      }
      mooseAssert(found_inner_sides == np, "Failed to find full boundary around meshed hole");

      // Retrieve subdomain name map from the mesh to be stitched and insert it into the main
      // subdomain map
      const auto & increment_subdomain_map = hole_mesh.get_subdomain_name_map();
      main_subdomain_map.insert(increment_subdomain_map.begin(), increment_subdomain_map.end());

      // We do not need the hole_bdy_id_filter anymore
      for (const auto & bcid : hole_bdy_id_filter)
        hole_boundary_info.remove_id(bcid);
      // If we are stitching a hole boundary layer mesh, we need to reassign the bcid
      if (hole_bdy_id_filter.size())
      {
        MooseMeshUtils::changeBoundaryId(
            hole_mesh,
            xyd_opts.hole_boundary_inner_id_defaults[hole_i].empty()
                ? 1
                : *xyd_opts.hole_boundary_inner_id_defaults[hole_i].begin(),
            hole_boundary_rec[hole_i],
            true);
        hole_mesh.get_boundary_info().sideset_name(hole_boundary_rec[hole_i]) =
            mesh->get_boundary_info().sideset_name(hole_boundary_rec[hole_i]);
        mesh->get_boundary_info().remove_id(hole_boundary_rec[hole_i]);
      }

      mesh->stitch_meshes(hole_mesh,
                          inner_bcid,
                          new_hole_bcid,
                          TOLERANCE,
                          /*clear_stitched_bcids*/ true,
                          xyd_opts.verbose_stitching,
                          xyd_opts.use_binary_search);
    }
  }
  // Check if one SubdomainName is shared by more than one subdomain ids
  std::set<SubdomainName> main_subdomain_map_name_list;
  for (auto const & id_name_pair : main_subdomain_map)
    main_subdomain_map_name_list.emplace(id_name_pair.second);
  if (main_subdomain_map.size() != main_subdomain_map_name_list.size())
    mg.paramError("holes", "The hole meshes contain subdomain name maps with conflicts.");

  mesh->unset_is_prepared();
  return mesh;
}
}
