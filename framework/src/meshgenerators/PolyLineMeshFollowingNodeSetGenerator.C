//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolyLineMeshFollowingNodeSetGenerator.h"

#include "CastUniquePointer.h"
#include "MooseMeshUtils.h"
#include "MooseUtils.h"

#include "libmesh/elem.h"
#include "libmesh/int_range.h"
#include "libmesh/unstructured_mesh.h"

registerMooseObject("MooseApp", PolyLineMeshFollowingNodeSetGenerator);

InputParameters
PolyLineMeshFollowingNodeSetGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  // Path parameters
  params.addRequiredParam<Point>("starting_point", "Starting point for the polyline");
  params.addRequiredParam<Point>("starting_direction",
                                 "Starting value for the direction of the line");
  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we get the sideset from");
  params.addRequiredParam<BoundaryName>("nodeset", "Nodeset to follow to form the polyline");
  params.addRequiredParam<Real>("search_radius",
                                "Radius of the sphere used to find points in the nodeset");
  params.addParam<bool>(
      "ignore_nodes_behind",
      false,
      "Ignore nodes in the nodeset that are behind the current point in the polyline");
  params.addParam<bool>("loop", false, "Whether edges should form a closed loop");

  // Discretization parameters
  // NOTE: we could have another dx as a path search parameter, and decouple the two options
  params.addRequiredRangeCheckedParam<Real>(
      "dx",
      "dx>0",
      "Approximate size of the edge elements (before any refinement with num_edges_between_points) "
      "and approximate step to advance by to search for the next point in the polyline");
  params.addParam<unsigned int>(
      "max_edges", 1000, "Maximum number of edges. Serves as a stopping criterion");
  params.addParam<unsigned int>(
      "num_edges_between_points", 1, "How many Edge elements to build between each point pair");

  // Naming of result parameters
  params.addParam<SubdomainName>("line_subdomain", "line", "Subdomain name for the line");
  params.addParam<BoundaryName>(
      "start_boundary", "start", "Boundary to assign to (non-looped) polyline start");
  params.addParam<BoundaryName>(
      "end_boundary", "end", "Boundary to assign to (non-looped) polyline end");

  params.addParam<bool>(
      "verbose",
      false,
      "whether to output additional information to console during the line generation");

  params.addClassDescription(
      "Generates a polyline (open ended or looped) of Edge elements by marching along a nodeeset "
      "and trying to be as close as possible to the nodes of the nodeset");

  return params;
}

PolyLineMeshFollowingNodeSetGenerator::PolyLineMeshFollowingNodeSetGenerator(
    const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _starting_point(getParam<Point>("starting_point")),
    _starting_direction(getParam<Point>("starting_direction")),
    _ignore_nodes_behind(getParam<bool>("ignore_nodes_behind")),
    _loop(getParam<bool>("loop")),
    _line_subdomain(getParam<SubdomainName>("line_subdomain")),
    _start_boundary(getParam<BoundaryName>("start_boundary")),
    _end_boundary(getParam<BoundaryName>("end_boundary")),
    _dx(getParam<Real>("dx")),
    _num_edges_between_points(getParam<unsigned int>("num_edges_between_points")),
    _verbose(getParam<bool>("verbose"))
{
  if (_loop && (isParamSetByUser("start_boundary") || isParamSetByUser("end_boundary")))
    paramError("loop",
               "Loop does not have a start or end boundary. These parameters must not be passed.");
}

std::unique_ptr<MeshBase>
PolyLineMeshFollowingNodeSetGenerator::generate()
{
  auto uptr_mesh = buildMeshBaseObject();
  MeshBase & mesh = *uptr_mesh;
  std::unique_ptr<MeshBase> base_mesh = std::move(_input);
  if (!base_mesh->is_serial())
    paramError("input", "Input mesh must not be distributed");

  // Get nodeset ID in input mesh
  const auto nodeset_id =
      MooseMeshUtils::getBoundaryID(getParam<BoundaryName>("nodeset"), *base_mesh);

  const auto search_radius_sq = libMesh::Utility::pow<2>(getParam<Real>("search_radius"));
  const auto n_points = getParam<unsigned int>("max_edges");
  Point current_point = _starting_point;
  Point previous_direction = _starting_direction;
  mesh.add_point(_starting_point, 0);

  // Pre-find all points in the nodeset to follow
  // TODO: Build a KNN tree to speed up the search later on
  const auto all_nodeset_tuples = base_mesh->get_boundary_info().build_node_list(
      libMesh::BoundaryInfo::NodeBCTupleSortBy::BOUNDARY_ID);
  std::vector<dof_id_type> nodeset_nodes;
  nodeset_nodes.reserve(all_nodeset_tuples.size() /
                        base_mesh->get_boundary_info().n_boundary_ids());
  for (const auto & tup : all_nodeset_tuples)
    if (BoundaryID(std::get<1>(tup)) == nodeset_id)
      nodeset_nodes.push_back(std::get<0>(tup));
  if (_verbose)
    _console << "Total number of nodes in nodeset " << getParam<BoundaryName>("nodeset") << ": "
             << nodeset_nodes.size() << std::endl;

  unsigned int n_segments = 0;
  for (const auto i : make_range(n_points))
  {
    // Move the point forward in the search direction
    const auto previous_point = current_point;
    current_point += previous_direction * _dx;

    // Draw a sphere to find the next point as the barycenter of the nodes from the nodeset inside
    // the sphere.
    // NOTE: there are many heuristics we could use here. We could try to fit a cylinder
    // to the nodeset if we know the nodeset represents a cylinder for example.
    Point barycenter(0);
    unsigned int n_sum = 0;
    for (const auto n_id : nodeset_nodes)
      if ((current_point - base_mesh->node_ref(n_id)).norm_sq() < search_radius_sq)
      {
        if (!_ignore_nodes_behind ||
            ((base_mesh->node_ref(n_id) - current_point) * previous_direction >= 0))
        {
          barycenter += base_mesh->node_ref(n_id);
          n_sum++;
        }
      }
    if (n_sum > 0)
      barycenter /= n_sum;
    else if (!_ignore_nodes_behind)
      mooseError("Did not find any nodes in the nodeset near the current point at: ",
                 current_point);
    else
      barycenter = previous_point;

    if (MooseUtils::absoluteFuzzyEqual((barycenter - previous_point).norm_sq(), 0))
    {
      mooseInfo("Barycenter did not move from ", barycenter, ". Returning!");
      goto done_drawing;
    }

    // Compute new direction
    const auto new_direction = (barycenter - previous_point).unit();
    previous_direction = new_direction;

    // Set the new point towards the barycenter
    n_segments++;
    // Note: this dx could be different than the dx used to search the barycenter
    current_point = previous_point + _dx * new_direction;
    mesh.add_point(current_point, (i + 1) * _num_edges_between_points);
    if (_verbose)
      _console << i << ": new point: " << current_point << " new direction " << previous_direction
               << std::endl;

    // Add the additional edges in between if requested
    if (_num_edges_between_points > 1)
    {
      auto p = previous_point;
      const Point pvec = (current_point - previous_point) / _num_edges_between_points;
      for (auto j : make_range(1u, _num_edges_between_points))
      {
        p += pvec;
        mesh.add_point(p, i * _num_edges_between_points + j);
      }
    }
  }

done_drawing:

  const auto n_elem = n_segments * _num_edges_between_points - 1;
  const auto max_nodes = n_segments * _num_edges_between_points - 1;
  for (auto i : make_range(n_elem + _loop))
  {
    const auto ip1 = _loop ? (i + 1) % max_nodes : (i + 1);
    auto elem = Elem::build(EDGE2);
    elem->set_node(0, mesh.node_ptr(i));
    elem->set_node(1, mesh.node_ptr(ip1));
    elem->set_id() = i;
    mesh.add_elem(std::move(elem));
  }

  // Add the starting and end boundary
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
