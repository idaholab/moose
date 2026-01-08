//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyLineMeshGenerator.h"

#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"
#include "libmesh/int_range.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", PolyLineMeshGenerator);

InputParameters
PolyLineMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addParam<std::vector<Point>>("points", "The points defining the polyline, in order");

  params.addParam<bool>("loop", false, "Whether edges should form a closed loop");

  params.addParam<BoundaryName>(
      "start_boundary", "start", "Boundary to assign to (non-looped) polyline start");

  params.addParam<BoundaryName>(
      "end_boundary", "end", "Boundary to assign to (non-looped) polyline end");

  params.addParam<std::vector<unsigned int>>(
      "nums_edges_between_points",
      {1},
      "How many Edge elements to build between each point pair. If a single value is given, it is "
      "applied to all segments. Otherwise, the number of entries must match the number of "
      "segments.");
  params.addDeprecatedParam<unsigned int>("num_edges_between_points",
                                          1,
                                          "How many Edge elements to build between each point pair",
                                          "Use nums_edges_between_points instead");

  params.addClassDescription("Generates meshes from edges connecting a list of points.");

  return params;
}

PolyLineMeshGenerator::PolyLineMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _points(getParam<std::vector<Point>>("points")),
    _loop(getParam<bool>("loop")),
    _start_boundary(_loop ? BoundaryName() : getParam<BoundaryName>("start_boundary")),
    _end_boundary(_loop ? BoundaryName() : getParam<BoundaryName>("end_boundary")),
    _nums_edges_between_points(
        parameters.isParamSetByUser("num_edges_between_points")
            ? std::vector<unsigned int>{getParam<unsigned int>("num_edges_between_points")}
            : getParam<std::vector<unsigned int>>("nums_edges_between_points"))
{
  if (_points.size() < 2)
    paramError("points", "At least 2 points are needed to define a polyline");

  if (_loop && _points.size() < 3)
    paramError("points", "At least 3 points are needed to define a polygon");

  if (_nums_edges_between_points.size() != 1 &&
      _nums_edges_between_points.size() != _points.size() - 1 + (_loop ? 1 : 0))
    paramError(
        "nums_edges_between_points",
        "This size of this vector input parameter must be 1 or match the number of segments");
}

std::unique_ptr<MeshBase>
PolyLineMeshGenerator::generate()
{
  auto uptr_mesh = buildMeshBaseObject();
  MeshBase & mesh = *uptr_mesh;

  MooseMeshUtils::buildPolyLineMesh(
      mesh, _points, _loop, _start_boundary, _end_boundary, _nums_edges_between_points);

  return uptr_mesh;
}
