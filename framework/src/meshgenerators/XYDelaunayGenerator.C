//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "XYDelaunayGenerator.h"

#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_modification.h"
#include "libmesh/mesh_serializer.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/parsed_function.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", XYDelaunayGenerator);

InputParameters
XYDelaunayGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum algorithm("BINARY EXHAUSTIVE", "BINARY");

  params.addRequiredParam<MeshGeneratorName>(
      "boundary", "The input MeshGenerator that defines the output mesh outer boundary.");
  params.addParam<std::vector<BoundaryName>>(
      "input_boundary_names", "2D-input-mesh boundaries defining the output mesh outer boundary");
  params.addParam<std::vector<SubdomainName>>(
      "input_subdomain_names", "1D-input-mesh subdomains defining the output mesh outer boundary");
  params.addParam<unsigned int>("add_nodes_per_boundary_segment",
                                0,
                                "How many more nodes to add in each outer boundary segment.");
  params.addParam<bool>(
      "refine_boundary", true, "Whether to allow automatically refining the outer boundary.");

  params.addParam<SubdomainName>("output_subdomain_name",
                                 "Subdomain name to set on new triangles.");

  params.addParam<BoundaryName>("output_boundary",
                                "Boundary name to set on new outer boundary.  Default ID 0.");
  params.addParam<std::vector<BoundaryName>>(
      "hole_boundaries", "Boundary names to set on holes.  Default IDs are numbered up from 1.");

  params.addParam<bool>(
      "verify_holes",
      true,
      "Verify holes do not intersect boundary or each other.  Asymptotically costly.");

  params.addParam<bool>("smooth_triangulation",
                        false,
                        "Whether to do Laplacian mesh smoothing on the generated triangles.");
  params.addParam<std::vector<MeshGeneratorName>>(
      "holes", std::vector<MeshGeneratorName>(), "The MeshGenerators that define mesh holes.");
  params.addParam<std::vector<bool>>(
      "stitch_holes", std::vector<bool>(), "Whether to stitch to the mesh defining each hole.");
  params.addParam<std::vector<bool>>("refine_holes",
                                     std::vector<bool>(),
                                     "Whether to allow automatically refining each hole boundary.");
  params.addRangeCheckedParam<Real>(
      "desired_area",
      0,
      "desired_area>=0",
      "Desired (maximum) triangle area, or 0 to skip uniform refinement");
  params.addParam<std::string>(
      "desired_area_func",
      std::string(),
      "Desired area as a function of x,y; omit to skip non-uniform refinement");
  params.addParam<MooseEnum>(
      "algorithm",
      algorithm,
      "Control the use of binary search for the nodes of the stitched surfaces.");
  params.addParam<bool>(
      "verbose_stitching", false, "Whether mesh stitching should have verbose output.");

  params.addClassDescription("Triangulates meshes within boundaries defined by input meshes.");

  return params;
}

XYDelaunayGenerator::XYDelaunayGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _bdy_ptr(getMesh("boundary")),
    _add_nodes_per_boundary_segment(getParam<unsigned int>("add_nodes_per_boundary_segment")),
    _refine_bdy(getParam<bool>("refine_boundary")),
    _output_subdomain_id(0),
    _smooth_tri(getParam<bool>("smooth_triangulation")),
    _verify_holes(getParam<bool>("verify_holes")),
    _hole_ptrs(getMeshes("holes")),
    _stitch_holes(getParam<std::vector<bool>>("stitch_holes")),
    _refine_holes(getParam<std::vector<bool>>("refine_holes")),
    _desired_area(getParam<Real>("desired_area")),
    _desired_area_func(getParam<std::string>("desired_area_func")),
    _algorithm(parameters.get<MooseEnum>("algorithm")),
    _verbose_stitching(parameters.get<bool>("verbose_stitching"))
{
  if (!_stitch_holes.empty() && _stitch_holes.size() != _hole_ptrs.size())
    paramError("stitch_holes", "Need one stitch_holes entry per hole, if specified.");

  for (auto hole_i : index_range(_stitch_holes))
    if (hole_i < _refine_holes.size() && _refine_holes[hole_i])
      paramError("refine_holes", "Disable auto refine of any hole boundary to be stitched.");
}

std::unique_ptr<MeshBase>
XYDelaunayGenerator::generate()
{
  // Put the boundary mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh =
      dynamic_pointer_cast<UnstructuredMesh>(std::move(_bdy_ptr));

  // Get ready to triangulate the line segments we extract from it
  Poly2TriTriangulator poly2tri(*mesh);
  poly2tri.triangulation_type() = TriangulatorInterface::PSLG;

  // If we're using a user-requested subset of boundaries on that
  // mesh, get their ids.
  std::set<std::size_t> bdy_ids;

  if (isParamValid("input_boundary_names"))
  {
    if (isParamValid("input_subdomain_names"))
      paramError(
          "input_subdomain_names",
          "input_boundary_names and input_subdomain_names cannot both specify an outer boundary.");

    const auto & boundary_names = getParam<std::vector<BoundaryName>>("input_boundary_names");
    for (const auto & name : boundary_names)
    {
      auto bcid = MooseMeshUtils::getBoundaryID(name, *mesh);
      if (bcid == BoundaryInfo::invalid_id)
        paramError("input_boundary_names", name, " is not a boundary name in the input mesh");

      bdy_ids.insert(bcid);
    }
  }

  if (isParamValid("input_subdomain_names"))
  {
    const auto & subdomain_names = getParam<std::vector<SubdomainName>>("input_subdomain_names");

    const auto subdomain_ids = MooseMeshUtils::getSubdomainIDs(*mesh, subdomain_names);

    // Check that the requested subdomains exist in the mesh
    std::set<SubdomainID> subdomains;
    mesh->subdomain_ids(subdomains);

    for (auto i : index_range(subdomain_ids))
    {
      if (subdomain_ids[i] == Moose::INVALID_BLOCK_ID || !subdomains.count(subdomain_ids[i]))
        paramError(
            "input_subdomain_names", subdomain_names[i], " was not found in the boundary mesh");

      bdy_ids.insert(subdomain_ids[i]);
    }
  }

  if (!bdy_ids.empty())
    poly2tri.set_outer_boundary_ids(bdy_ids);

  poly2tri.set_interpolate_boundary_points(_add_nodes_per_boundary_segment);
  poly2tri.set_refine_boundary_allowed(_refine_bdy);
  poly2tri.set_verify_hole_boundaries(_verify_holes);

  poly2tri.desired_area() = _desired_area;
  poly2tri.minimum_angle() = 0; // Not yet supported
  poly2tri.smooth_after_generating() = _smooth_tri;

  if (_desired_area_func != "")
  {
    // poly2tri will clone this so it's fine going out of scope
    ParsedFunction<Real> area_func{_desired_area_func};
    poly2tri.set_desired_area_function(&area_func);
  }

  std::vector<TriangulatorInterface::MeshedHole> meshed_holes;
  std::vector<TriangulatorInterface::Hole *> triangulator_hole_ptrs(_hole_ptrs.size());
  std::vector<std::unique_ptr<MeshBase>> hole_ptrs(_hole_ptrs.size());

  // Make sure pointers here aren't invalidated by a resize
  meshed_holes.reserve(_hole_ptrs.size());
  for (auto hole_i : index_range(_hole_ptrs))
  {
    hole_ptrs[hole_i] = std::move(*_hole_ptrs[hole_i]);
    meshed_holes.emplace_back(*hole_ptrs[hole_i]);
    if (hole_i < _refine_holes.size())
      meshed_holes.back().set_refine_boundary_allowed(_refine_holes[hole_i]);

    triangulator_hole_ptrs[hole_i] = &meshed_holes.back();
  }

  if (!triangulator_hole_ptrs.empty())
    poly2tri.attach_hole_list(&triangulator_hole_ptrs);

  poly2tri.triangulate();

  if (isParamValid("output_subdomain_name"))
  {
    auto output_subdomain_name = getParam<SubdomainName>("output_subdomain_name");
    _output_subdomain_id = MooseMeshUtils::getSubdomainID(output_subdomain_name, *mesh);

    if (_output_subdomain_id == Elem::invalid_subdomain_id)
    {
      // We'll probably need to make a new ID, then
      _output_subdomain_id = MooseMeshUtils::getNextFreeSubdomainID(*mesh);

      // But check the hole meshes for our output subdomain name too
      for (auto & hole_ptr : hole_ptrs)
      {
        auto possible_sbdid = MooseMeshUtils::getSubdomainID(output_subdomain_name, *hole_ptr);
        // Huh, it was in one of them
        if (possible_sbdid != Elem::invalid_subdomain_id)
        {
          _output_subdomain_id = possible_sbdid;
          break;
        }
        _output_subdomain_id =
            std::max(_output_subdomain_id, MooseMeshUtils::getNextFreeSubdomainID(*hole_ptr));
      }

      mesh->subdomain_name(_output_subdomain_id) = output_subdomain_name;
    }
  }

  if (_smooth_tri || _output_subdomain_id)
    for (auto elem : mesh->element_ptr_range())
    {
      mooseAssert(elem->type() == TRI3, "Unexpected non-Tri3 found in triangulation");

      elem->subdomain_id() = _output_subdomain_id;

      // I do not trust Laplacian mesh smoothing not to invert
      // elements near reentrant corners.  Eventually we'll add better
      // smoothing options, but even those might have failure cases.
      // Better to always do extra tests here than to ever let users
      // try to run on a degenerate mesh.
      if (_smooth_tri)
      {
        auto cross_prod = (elem->point(1) - elem->point(0)).cross(elem->point(2) - elem->point(0));

        if (cross_prod(2) <= 0)
          mooseError("Inverted element found in triangulation.\n"
                     "Laplacian smoothing can create these at reentrant corners; disable it?");
      }
    }

  // Assign new subdomain name, if provided
  if (isParamValid("output_subdomain_name"))
    mesh->subdomain_name(_output_subdomain_id) = getParam<SubdomainName>("output_subdomain_name");

  const bool use_binary_search = (_algorithm == "BINARY");

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
  const boundary_id_type end_bcid = hole_ptrs.size() + 1;
  boundary_id_type new_hole_bcid = end_bcid;

  // We might be overriding the default bcid numbers.  We have to be
  // careful about how we renumber, though.  We pick unused temporary
  // numbers because e.g. "0->2, 2->0" is impossible to do
  // sequentially, but "0->N, 2->N+2, N->2, N+2->0" works.
  if (isParamValid("output_boundary"))
    libMesh::MeshTools::Modification::change_boundary_id(*mesh, 0, end_bcid);

  if (isParamValid("hole_boundaries"))
  {
    auto hole_boundaries = getParam<std::vector<BoundaryName>>("hole_boundaries");
    auto hole_boundary_ids = MooseMeshUtils::getBoundaryIDs(*mesh, hole_boundaries, true);

    if (hole_boundary_ids.size() != hole_ptrs.size())
      paramError("hole_boundary_ids", "Need one hole_boundary_ids entry per hole, if specified.");

    for (auto h : index_range(hole_ptrs))
      libMesh::MeshTools::Modification::change_boundary_id(*mesh, h + 1, h + 1 + end_bcid);

    for (auto h : index_range(hole_ptrs))
    {
      libMesh::MeshTools::Modification::change_boundary_id(
          *mesh, h + 1 + end_bcid, hole_boundary_ids[h]);
      mesh->get_boundary_info().sideset_name(hole_boundary_ids[h]) = hole_boundaries[h];
      new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(hole_boundary_ids[h] + 1));
    }
  }

  if (isParamValid("output_boundary"))
  {
    const BoundaryName output_boundary = getParam<BoundaryName>("output_boundary");
    const std::vector<BoundaryID> output_boundary_id =
        MooseMeshUtils::getBoundaryIDs(*mesh, {output_boundary}, true);

    libMesh::MeshTools::Modification::change_boundary_id(*mesh, end_bcid, output_boundary_id[0]);
    mesh->get_boundary_info().sideset_name(output_boundary_id[0]) = output_boundary;

    new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(output_boundary_id[0] + 1));
  }

  bool doing_stitching = false;

  for (auto hole_i : index_range(hole_ptrs))
  {
    const MeshBase & hole_mesh = *hole_ptrs[hole_i];
    auto & hole_boundary_info = hole_mesh.get_boundary_info();
    const std::set<boundary_id_type> & local_hole_bcids = hole_boundary_info.get_boundary_ids();

    if (!local_hole_bcids.empty())
      new_hole_bcid = std::max(new_hole_bcid, boundary_id_type(*local_hole_bcids.rbegin() + 1));
    hole_mesh.comm().max(new_hole_bcid);

    if (hole_i < _stitch_holes.size() && _stitch_holes[hole_i])
      doing_stitching = true;
  }

  const boundary_id_type inner_bcid = new_hole_bcid + 1;

  // libMesh mesh stitching still requires a serialized mesh, and it's
  // cheaper to do that once than to do it once-per-hole
  MeshSerializer serial(*mesh, doing_stitching);

  for (auto hole_i : index_range(hole_ptrs))
  {
    if (hole_i < _stitch_holes.size() && _stitch_holes[hole_i])
    {
      UnstructuredMesh & hole_mesh = dynamic_cast<UnstructuredMesh &>(*hole_ptrs[hole_i]);
      auto & hole_boundary_info = hole_mesh.get_boundary_info();

      // Our algorithm here requires a serialized Mesh.  To avoid
      // redundant serialization and deserialization (libMesh
      // MeshedHole and stitch_meshes still also require
      // serialization) we'll do the serialization up front.
      MeshSerializer serial_hole(hole_mesh);

      // It would have been nicer for MeshedHole to add the BCID
      // itself, but we want MeshedHole to work with a const mesh.
      // We'll still use MeshedHole, for its code distinguishing
      // outer boundaries from inner boundaries on a
      // hole-with-holes.
      TriangulatorInterface::MeshedHole mh{hole_mesh};

      // We have to translate from MeshedHole points to mesh
      // sides.
      std::unordered_map<Point, Point> next_hole_boundary_point;
      const int np = mh.n_points();
      for (auto pi : make_range(1, np))
        next_hole_boundary_point[mh.point(pi - 1)] = mh.point(pi);
      next_hole_boundary_point[mh.point(np - 1)] = mh.point(0);

      int found_hole_sides = 0;
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
              ++found_hole_sides;
            }
        }
      }
      mooseAssert(found_hole_sides == np, "Failed to find full outer boundary of meshed hole");

      auto & mesh_boundary_info = mesh->get_boundary_info();
      int found_inner_sides = 0;
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
              ++found_inner_sides;
            }
        }
      }
      mooseAssert(found_inner_sides == np, "Failed to find full boundary around meshed hole");

      mesh->stitch_meshes(hole_mesh,
                          inner_bcid,
                          new_hole_bcid,
                          TOLERANCE,
                          /*clear_stitched_bcids*/ true,
                          _verbose_stitching,
                          use_binary_search);
    }
  }
  return mesh;
}
