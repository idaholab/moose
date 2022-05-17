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
  params.addParam<std::vector<MeshGeneratorName>>(
      "holes", std::vector<MeshGeneratorName>(), "The MeshGenerators that define mesh holes.");
  params.addParam<std::vector<bool>>(
      "stitch_holes", std::vector<bool>(), "Whether to stitch to the mesh defining each hole.");
  params.addParam<std::vector<unsigned int>>("interpolate_holes",
                                             std::vector<unsigned int>(),
                                             "How many nodes to use per hole boundary segment.");
  params.addParam<std::vector<bool>>("refine_holes",
                                     std::vector<bool>(),
                                     "Whether to allow automatically refining each hole boundary.");
  params.addParam<Real>("desired_area", std::numeric_limits<Real>::max(), "Desired triangle area.");
  params.addParam<std::string>("desired_area_func",
                               std::string(),
                               "Function specifying desired triangle area as a function of x,y.");
  params.addParam<MooseEnum>(
      "algorithm",
      algorithm,
      "Control the use of binary search for the nodes of the stitched surfaces.");

  return params;
}

Poly2TriMeshGenerator::Poly2TriMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _bdy_ptr(getMesh("boundary")),
    _interpolate_bdy(getParam<unsigned int>("interpolate_boundary")),
    _refine_bdy(getParam<bool>("refine_boundary")),
    _hole_ptrs(getMeshes("holes")),
    _stitch_holes(getParam<std::vector<bool>>("stitch_holes")),
    _interpolate_holes(getParam<std::vector<unsigned int>>("interpolate_holes")),
    _refine_holes(getParam<std::vector<bool>>("refine_holes")),
    _desired_area(getParam<Real>("desired_area")),
    _desired_area_func(getParam<std::string>("desired_area_func")),
    _algorithm(parameters.get<MooseEnum>("algorithm"))
{
}

std::unique_ptr<MeshBase>
Poly2TriMeshGenerator::generate()
{
  // We put the boundary mesh in a local pointer
  std::unique_ptr<UnstructuredMesh> mesh = dynamic_pointer_cast<UnstructuredMesh>(_bdy_ptr);

  if (!_stitch_holes.empty() && _stitch_holes.size() != _hole_ptrs.size())
    paramError("stitch_holes", "Need one stitch_holes entry per hole, if specified.");

  if (!_interpolate_holes.empty() && _interpolate_holes.size() != _hole_ptrs.size())
    paramError("interpolate_holes", "Need one interpolate_holes entry per hole, if specified.");

  Poly2TriTriangulator poly2tri(*mesh);
  poly2tri.triangulation_type() = TriangulatorInterface::PSLG;

  poly2tri.set_interpolate_boundary_points(_interpolate_bdy);
  poly2tri.set_refine_boundary_allowed(_refine_bdy);

  poly2tri.desired_area() = _desired_area;
  poly2tri.minimum_angle() = 0; // Not yet supported
  poly2tri.smooth_after_generating() = false; // Unsafe on concave domains IMHO

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
    if (_stitch_holes.size() > hole_i)
      libmesh_not_implemented();

    triangulator_hole_ptrs.push_back(&meshed_holes.back());
  }

  if (!triangulator_hole_ptrs.empty())
    poly2tri.attach_hole_list(&triangulator_hole_ptrs);

  poly2tri.triangulate();

  return mesh;
}
