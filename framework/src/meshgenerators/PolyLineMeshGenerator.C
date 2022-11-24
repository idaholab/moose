//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

  params.addParam<unsigned int>(
      "num_edges_between_points", 1, "How many Edge elements to build between each point pair");

  params.addClassDescription("Generates meshes from edges connecting a list of points.");

  return params;
}

PolyLineMeshGenerator::PolyLineMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _points(getParam<std::vector<Point>>("points")),
    _loop(getParam<bool>("loop")),
    _start_boundary(getParam<BoundaryName>("start_boundary")),
    _end_boundary(getParam<BoundaryName>("end_boundary")),
    _num_edges_between_points(getParam<unsigned int>("num_edges_between_points"))
{
  if (_points.size() < 2)
    paramError("points", "At least 2 points are needed to define a polyline");

  if (_loop && _points.size() < 3)
    paramError("points", "At least 3 points are needed to define a polygon");
}

std::unique_ptr<MeshBase>
PolyLineMeshGenerator::generate()
{
  auto uptr_mesh = buildMeshBaseObject();
  MeshBase & mesh = *uptr_mesh;

  const auto n_points = _points.size();
  for (auto i : make_range(n_points))
  {
    Point p = _points[i];
    mesh.add_point(p, i * _num_edges_between_points);
    if (_num_edges_between_points > 1)
    {
      if (!_loop && (i + 1) == n_points)
        break;

      const auto ip1 = (i + 1) % n_points;
      const Point pvec = (_points[ip1] - p) / _num_edges_between_points;

      for (auto j : make_range(1u, _num_edges_between_points))
      {
        p += pvec;
        mesh.add_point(p, i * _num_edges_between_points + j);
      }
    }
  }

  const auto n_segments = _loop ? n_points : (n_points - 1);
  const auto n_elem = n_segments * _num_edges_between_points;
  const auto max_nodes = n_points * _num_edges_between_points;
  for (auto i : make_range(n_elem))
  {
    const auto ip1 = (i + 1) % max_nodes;
    auto elem = Elem::build(EDGE2);
    elem->set_node(0) = mesh.node_ptr(i);
    elem->set_node(1) = mesh.node_ptr(ip1);
    elem->set_id() = i;
    mesh.add_elem(std::move(elem));
  }

  if (!_loop)
  {
    BoundaryInfo & bi = mesh.get_boundary_info();
    std::vector<BoundaryName> bdy_names{_start_boundary, _end_boundary};
    std::vector<boundary_id_type> ids = MooseMeshUtils::getBoundaryIDs(mesh, bdy_names, true);
    bi.add_side(mesh.elem_ptr(0), 0, ids[0]);
    bi.add_side(mesh.elem_ptr(n_elem - 1), 1, ids[1]);
  }

  mesh.prepare_for_use();

  return uptr_mesh;
}
