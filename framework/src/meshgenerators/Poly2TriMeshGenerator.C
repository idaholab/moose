//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Poly2TriMeshGenerator.h"

#include "CastUniquePointer.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"
#include "libmesh/int_range.h"
#include "libmesh/mesh_triangle_holes.h"
#include "libmesh/parsed_function.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", Poly2TriMeshGenerator);

InputParameters
Poly2TriMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  MooseEnum algorithm("BINARY EXHAUSTIVE", "BINARY");

  params.addRequiredParam<MeshGeneratorName>(
      "boundary", "The MeshGenerator that defines the mesh outer boundary.");
  params.addParam<unsigned int>(
      "interpolate_boundary", 0, "How many more nodes to add in each outer boundary segment.");
  params.addParam<bool>(
      "refine_boundary", true, "Whether to allow automatically refining the outer boundary.");
  params.addParam<bool>("smooth_triangulation",
                        false,
                        "Whether to do Laplacian mesh smoothing on the generated triangles.");
  params.addParam<std::vector<MeshGeneratorName>>(
      "holes", std::vector<MeshGeneratorName>(), "The MeshGenerators that define mesh holes.");
  params.addParam<std::vector<bool>>(
      "stitch_holes", std::vector<bool>(), "Whether to stitch to the mesh defining each hole.");
  params.addParam<std::vector<unsigned int>>("interpolate_holes",
                                             std::vector<unsigned int>(),
                                             "How many nodes to add per hole boundary segment.");
  params.addParam<std::vector<bool>>("refine_holes",
                                     std::vector<bool>(),
                                     "Whether to allow automatically refining each hole boundary.");
  params.addParam<Real>(
      "desired_area", std::numeric_limits<Real>::max(), "Desired (maximum) triangle area.");
  params.addParam<std::string>("desired_area_func",
                               std::string(),
                               "Function specifying desired triangle area as a function of x,y.");
  params.addParam<MooseEnum>(
      "algorithm",
      algorithm,
      "Control the use of binary search for the nodes of the stitched surfaces.");
  params.addParam<bool>(
      "verbose_stitching", false, "Whether mesh stitching should have verbose output.");

  params.addClassDescription("Triangulates meshes within boundaries defined by input meshes.");

  return params;
}

Poly2TriMeshGenerator::Poly2TriMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _bdy_ptr(getMesh("boundary")),
    _interpolate_bdy(getParam<unsigned int>("interpolate_boundary")),
    _refine_bdy(getParam<bool>("refine_boundary")),
    _smooth_tri(getParam<bool>("smooth_triangulation")),
    _hole_ptrs(getMeshes("holes")),
    _stitch_holes(getParam<std::vector<bool>>("stitch_holes")),
    _interpolate_holes(getParam<std::vector<unsigned int>>("interpolate_holes")),
    _refine_holes(getParam<std::vector<bool>>("refine_holes")),
    _desired_area(getParam<Real>("desired_area")),
    _desired_area_func(getParam<std::string>("desired_area_func")),
    _algorithm(parameters.get<MooseEnum>("algorithm")),
    _verbose_stitching(parameters.get<bool>("verbose_stitching"))
{
  if (!_stitch_holes.empty() && _stitch_holes.size() != _hole_ptrs.size())
    paramError("stitch_holes", "Need one stitch_holes entry per hole, if specified.");

  if (!_interpolate_holes.empty() && _interpolate_holes.size() != _hole_ptrs.size())
    paramError("interpolate_holes", "Need one interpolate_holes entry per hole, if specified.");
}

std::unique_ptr<MeshBase>
Poly2TriMeshGenerator::generate()
{
  // We put the boundary mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh = dynamic_pointer_cast<UnstructuredMesh>(_bdy_ptr);

  Poly2TriTriangulator poly2tri(*mesh);
  poly2tri.triangulation_type() = TriangulatorInterface::PSLG;

  poly2tri.set_interpolate_boundary_points(_interpolate_bdy);
  poly2tri.set_refine_boundary_allowed(_refine_bdy);

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
  std::vector<TriangulatorInterface::Hole *> triangulator_hole_ptrs;

  // Make sure pointers here aren't invalidated by a resize
  meshed_holes.reserve(_hole_ptrs.size());
  for (auto hole_i : index_range(_hole_ptrs))
  {
    meshed_holes.emplace_back(*_hole_ptrs[hole_i]->get());
    if (hole_i < _refine_holes.size())
      meshed_holes.back().set_refine_boundary_allowed(_refine_holes[hole_i]);
    if (_interpolate_holes.size() > hole_i)
      libmesh_not_implemented();

    triangulator_hole_ptrs.push_back(&meshed_holes.back());
  }

  if (!triangulator_hole_ptrs.empty())
    poly2tri.attach_hole_list(&triangulator_hole_ptrs);

  poly2tri.triangulate();

  // I do not trust Laplacian mesh smoothing not to invert elements
  // near reentrant corners.  Eventually we'll add better smoothing
  // options, but even those might have failure cases.  Better to
  // always do an extra loop here than to ever let users try to run on
  // a degenerate mesh.
  if (_smooth_tri)
    for (auto elem : mesh->element_ptr_range())
    {
      mooseAssert(elem->type() == TRI3, "Unexpected non-Tri3 found in triangulation");
      auto cross_prod = (elem->point(1) - elem->point(0)).cross(elem->point(2) - elem->point(0));

      if (cross_prod(2) <= 0)
        mooseError("Inverted element found in triangulation.\n"
                   "Laplacian smoothing can create these at reentrant corners; disable it?");
    }

  const bool use_binary_search = (_algorithm == "BINARY");

  // The hole meshes are specified by the user, so they could have any
  // BCID or no BCID or any combination of BCIDs on their outer
  // boundary, so we'll have to set our own BCID to use for stitching
  // there.  We'll need to check all the holes for used BCIDs, if we
  // want to pick a new ID on hole N that doesn't conflict with any
  // IDs on hole M < N (or with the IDs on the new triangulation)

  // The new triangulation assigns BCID i+1 to hole i ... but we can't
  // even use this for mesh stitching, because we can't be sure it
  // isn't also already in use on the hole's mesh and so we won't be
  // able to safely clear it afterwards.

  boundary_id_type new_hole_bcid = _hole_ptrs.size();
  for (auto hole_i : index_range(_hole_ptrs))
  {
    const MeshBase & hole_mesh = **_hole_ptrs[hole_i];
    auto & hole_boundary_info = hole_mesh.get_boundary_info();
    const std::set<boundary_id_type> & local_hole_bcids = hole_boundary_info.get_boundary_ids();

    if (!local_hole_bcids.empty())
      new_hole_bcid = std::max(new_hole_bcid, *local_hole_bcids.rbegin());
    hole_mesh.comm().max(new_hole_bcid);
  }

  new_hole_bcid++;
  const boundary_id_type inner_bcid = new_hole_bcid + 1;

  for (auto hole_i : index_range(_hole_ptrs))
  {
    if (hole_i < _stitch_holes.size() && _stitch_holes[hole_i])
    {
      UnstructuredMesh & hole_mesh = dynamic_cast<UnstructuredMesh &>(*_hole_ptrs[hole_i]->get());
      auto & hole_boundary_info = hole_mesh.get_boundary_info();

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
        auto ns = elem->n_sides();
        for (auto s : make_range(ns))
        {
          // Remember that the point ordering should be
          // reversed since we're thinking of it as an outer
          // boundary rather than an inner here.
          auto it_s = next_hole_boundary_point.find(elem->point((s + 1) % ns));
          if (it_s != next_hole_boundary_point.end())
            if (it_s->second == elem->point(s))
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
          auto it_s = next_hole_boundary_point.find(elem->point(s));
          if (it_s != next_hole_boundary_point.end())
            if (it_s->second == elem->point((s + 1) % ns))
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
