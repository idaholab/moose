//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeripheralTriangleMeshGenerator.h"

// Moose headers
#include "CastUniquePointer.h" // dynamic_pointer_cast, but not std::

// libMesh headers - some of this only has forward declares so far
#include "libmesh/boundary_info.h"
#include "libmesh/elem.h"
#include "libmesh/face_tri3.h"
#include "libmesh/mesh_base.h"

// poly2tri triangulation library
#include "poly2tri/poly2tri.h"

// Anonymous namespace - poly2tri doesn't define operator<(Point,Point)
namespace
{
struct P2TPointCompare
{
  bool operator()(const p2t::Point & a, const p2t::Point & b) const
  {
    return a.x < b.x || (a.x == b.x && a.y < b.y);
  }
};
}

registerMooseObject("ReactorApp", PeripheralTriangleMeshGenerator);

InputParameters
PeripheralTriangleMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRangeCheckedParam<SubdomainID>(
      "subdomain_id",
      "subdomain_id>=0",
      "The subdomain ID given to the TRI3 elements in the triangulation.");
  params.addRequiredParam<MeshGeneratorName>(
      "inner_boundary_mesh", "The mesh to use for the inner boundary of the triangulation.");
  params.addRangeCheckedParam<boundary_id_type>(
      "inner_boundary_id",
      "inner_boundary_id>=0",
      "The boundary ID of the inner mesh outer boundary.");
  params.addParam<std::string>("inner_boundary_name",
                               "The boundary name of the inner mesh outer boundary.");
  params.addRequiredRangeCheckedParam<Real>(
      "outer_circle_radius", "outer_circle_radius>0", "Radius of outer circle boundary.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "outer_circle_num_segments",
      "outer_circle_num_segments>0",
      "Number of radial segments to subdivide outer circle boundary.");
  params.addRangeCheckedParam<boundary_id_type>(
      "outer_boundary_id",
      "outer_boundary_id>=0",
      "The boundary id for the generated mesh outer boundary.");
  params.addParam<std::vector<Real>>("extra_circle_radii", "Radii of extra Steiner point circles.");
  params.addParam<std::vector<unsigned int>>("extra_circle_num_segments",
                                             "Number of segments for extra Steiner point circles.");
  params.addClassDescription("This PeripheralTriangleMeshGenerator object is designed to generate "
                             "a triangulated mesh between a generated outer circle boundary "
                             "and a provided inner mesh.");

  return params;
}

PeripheralTriangleMeshGenerator::PeripheralTriangleMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _tri_subdomain_id(isParamValid("subdomain_id") ? getParam<SubdomainID>("subdomain_id") : 0),
    _inner_boundary_mesh(getMesh("inner_boundary_mesh")),
    _outer_circle_radius(getParam<Real>("outer_circle_radius")),
    _outer_circle_num_segments(getParam<unsigned int>("outer_circle_num_segments")),
    _outer_boundary_id(
        isParamValid("outer_boundary_id") ? getParam<boundary_id_type>("outer_boundary_id") : 0),
    _extra_circle_radii(getParam<std::vector<Real>>("extra_circle_radii")),
    _extra_circle_num_segments(getParam<std::vector<unsigned int>>("extra_circle_num_segments"))
{
  // confirm either id or name (exclusive or) for inner mesh boundary is provided
  if (isParamValid("inner_boundary_id") == isParamValid("inner_boundary_name"))
  {
    paramError(
        "inner_boundary_id",
        "Either 'inner_boundary_id' or 'inner_boundary_name' must be provided, but not both.");
  }

  // confirm Steiner inputs are equal length
  if (_extra_circle_radii.size() != _extra_circle_num_segments.size())
  {
    paramError("extra_circle_num_segments",
               "The size of 'extra_circle_num_segments' must be equal to the size of "
               "'extra_circle_radii'.");
  }
}

std::unique_ptr<MeshBase>
PeripheralTriangleMeshGenerator::generate()
{
  //
  // Set up mesh based on input inner mesh
  //
  std::unique_ptr<MeshBase> mesh = std::move(_inner_boundary_mesh);

  //
  // The input mesh might be distributed (because a user wants a
  // distributed mesh in the end), but we'll need to serialize it to
  // do our work here.  If we're running distributed because we'll be
  // extruded later into 3D then it's probably still cheap enough for
  // us to serialize now.  If we're already serial we're fine.
  //
  if (!mesh->is_serial())
    mesh->allgather();

  BoundaryInfo & boundary_info = mesh->get_boundary_info();
  const std::set<boundary_id_type> & boundary_ids = boundary_info.get_boundary_ids();

  // set inner boundary id based on give input
  if (isParamValid("inner_boundary_id"))
  {
    _inner_boundary_id = getParam<boundary_id_type>("inner_boundary_id");
    // confirm inner_boundary_id exists in input mesh
    if (boundary_ids.find(_inner_boundary_id) == boundary_ids.end())
    {
      paramError("inner_boundary_id", "Boundary id not found in input mesh.");
    }
  }
  if (isParamValid("inner_boundary_name"))
  {
    _inner_boundary_id = boundary_info.get_id_by_name(getParam<std::string>("inner_boundary_name"));
    // confirm inner_boundary_name exists in input mesh
    if (boundary_ids.find(_inner_boundary_id) == boundary_ids.end())
    {
      paramError("inner_boundary_name", "Boundary name not found in input mesh.");
    }
  }

  // build nodesets and sidesets
  boundary_info.build_node_list_from_side_list();
  boundary_info.build_side_list_from_node_list();

  //
  // extract boundary nodes from input mesh inner boundary
  // poly2tri requires input boundary nodes be sorted in connected order
  //

  std::vector<Node *> inner_boundary_nodes = createSortedBoundaryNodeList(*mesh);

  //
  // C2T storage
  //

  /// Polylines
  std::vector<p2t::Point> inner_polyline;
  std::vector<p2t::Point> outer_polyline;
  std::vector<p2t::Point> steiner_points;

  // store association map for reference
  std::map<const p2t::Point, Node *, P2TPointCompare> point_node_map;

  //
  // Inner P2T boundary; extract from existing mesh
  //

  // POLYLINE NEEDS TO BE DIRECTED

  for (Node * node : inner_boundary_nodes)
  {
    // extract (x, y) coords
    Real x = (*node)(0);
    Real y = (*node)(1);
    // add to inner boundary list
    inner_polyline.emplace_back(x, y);
    // add to association map
    point_node_map[inner_polyline.back()] = node;
  }

  //
  // Outer P2T boundary; create circle from input parameters
  //

  // radial spacing
  Real d_theta = 2.0 * M_PI / _outer_circle_num_segments;

  for (unsigned int i = 0; i < _outer_circle_num_segments; i++)
  {
    // rotation angle
    Real theta = i * d_theta;
    // calculate (x, y) coords
    Real x = _outer_circle_radius * std::cos(theta);
    Real y = _outer_circle_radius * std::sin(theta);

    // add to outer boundary list
    outer_polyline.emplace_back(x, y);
    // create Node and add to mesh
    Node * node = mesh->add_point(Point(x, y, 0.0));
    // add to association map
    point_node_map[outer_polyline.back()] = node;
  }

  //
  // Additional Steiner points; create additional circles of Steiner points between inner and outer
  // boundaries.
  //

  for (std::vector<unsigned int>::size_type idx = 0; idx < _extra_circle_radii.size(); idx++)
  {
    // extract Steiner point circle parameters
    Real num_segments = _extra_circle_num_segments[idx];
    Real radius = _extra_circle_radii[idx];

    // radial spacing
    d_theta = 2.0 * M_PI / num_segments;

    for (unsigned int i = 0; i < num_segments; i++)
    {
      // rotation angle
      Real theta = i * d_theta;
      // calculate (x, y) coords
      Real x = radius * std::cos(theta);
      Real y = radius * std::sin(theta);

      // add to Steiner point list
      steiner_points.emplace_back(x, y);
      // create Node and add to mesh
      Node * node = mesh->add_point(Point(x, y, 0.0));
      // add to association map
      point_node_map[steiner_points.back()] = node;
    }
  }

  //
  // Create P2T CDT and add primary (outer) polyline
  // NOTE: polyline must be a simple polygon. The polyline's points constitute constrained edges. No
  // repeat points!!!
  //

  std::vector<p2t::Point *> outer_polyline_ptrs(outer_polyline.size());
  std::transform(outer_polyline.begin(),
                 outer_polyline.end(),
                 outer_polyline_ptrs.begin(),
                 [](p2t::Point & p) { return &p; });
  p2t::CDT cdt{outer_polyline_ptrs};

  // Add inner boundary
  std::vector<p2t::Point *> inner_polyline_ptrs(inner_polyline.size());
  std::transform(inner_polyline.begin(),
                 inner_polyline.end(),
                 inner_polyline_ptrs.begin(),
                 [](p2t::Point & p) { return &p; });
  cdt.AddHole(inner_polyline_ptrs);

  // Add Steiner points
  for (auto & point : steiner_points)
    cdt.AddPoint(&point);

  // Triangulate!
  cdt.Triangulate();

  //
  // Translate triangulated C2T points/triangles to libMesh nodes/Tri3s
  //

  // Extract triangle list from triangulation
  std::vector<p2t::Triangle *> triangles = cdt.GetTriangles();

  // process each P2T triangle into libMesh Tri3
  for (p2t::Triangle * triangle : triangles)
  {

    // create Tri3 element
    Tri3 * tri_elem = new Tri3();

    // process each point on triangle
    for (const int i : std::vector<int>{0, 1, 2})
    {
      // extract p2t::point from triangle
      p2t::Point * point = triangle->GetPoint(i);

      // check if node has been created for point
      // if not, create and add to map
      if (!point_node_map.count(*point))
        point_node_map[*point] = mesh->add_point(Point(point->x, point->y, 0.0));

      // retrieve node and add to element
      tri_elem->set_node(i) = point_node_map[*point];
    }

    // set element subdomain id
    tri_elem->subdomain_id() = _tri_subdomain_id;

    // add completed element to mesh
    mesh->add_elem(tri_elem);
  }

  //
  // add nodesets for boundaries
  //

  for (const p2t::Point & point : outer_polyline)
    boundary_info.add_node(point_node_map[point], _outer_boundary_id);

  //
  // finalize mesh (partition the new elements, make the mesh
  // distributed again if necessary) and return
  //

  mesh->prepare_for_use();

  return dynamic_pointer_cast<MeshBase>(mesh);
}

std::vector<Node *>
PeripheralTriangleMeshGenerator::createSortedBoundaryNodeList(MeshBase & mesh) const
{
  std::vector<Node *> inner_boundary_nodes;

  BoundaryInfo & boundary_info = mesh.get_boundary_info();

  // poly2tri requires input boundary nodes be in connected order
  // create edge node pairs to sort by connectivity

  // vector of <element_id, side_id, boundary_id> tuples
  auto side_list = boundary_info.build_side_list();

  std::vector<std::pair<dof_id_type, dof_id_type>> boundary_node_assm;

  // create edge node pairs for sides on the specified boundary
  for (const auto & entry : side_list)
  {
    // select out tuple components
    dof_id_type element_id = std::get<0>(entry);
    unsigned short int side_id = std::get<1>(entry);
    boundary_id_type boundary_id = std::get<2>(entry);

    // skip sides not in given boundary id
    if (boundary_id != _inner_boundary_id)
    {
      continue;
    }

    // select out nodes for given side and add to list
    auto side = mesh.elem_ptr(element_id)->side_ptr(side_id);

    boundary_node_assm.push_back(std::make_pair(side->node_id(0), side->node_id(1)));
  }

  //
  // sort boundary sides into connected order
  //

  std::vector<dof_id_type> boundary_ordered_node_list;
  bool isFlipped = false;

  // check that boundary list is non-zero, otherwise error
  if (boundary_node_assm.size() == 0)
  {
    mooseError("In PeripheralTriangleMeshGenerator, inner boundary node list appears empty, ",
               "check inner boundary input.");
  }

  // start sorted node list with nodes from first side in side list, first two nodes are sorted
  // remove side from side list to be sorted
  boundary_ordered_node_list.push_back(boundary_node_assm.front().first);
  boundary_ordered_node_list.push_back(boundary_node_assm.front().second);
  boundary_node_assm.erase(boundary_node_assm.begin());

  // save initial length of the list, size will shrink during loop
  const unsigned int boundary_node_assm_size_0 = boundary_node_assm.size();
  // iterate over all remaining side pairs
  for (unsigned int i = 0; i < boundary_node_assm_size_0; i++)
  {
    // use the last node in the sorted node list for lookup
    // find the side in the remaining side list that connects to this node
    dof_id_type end_node_id = boundary_ordered_node_list.back();

    // create lambda functions to search remaining side list
    // lambda to check if current end_node_id is in pair first position
    auto node_in_first_pos = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair)
    { return old_id_pair.first == end_node_id; };
    // lambda to check if current end_node_id is in pair second position
    auto node_in_second_pos = [end_node_id](std::pair<dof_id_type, dof_id_type> old_id_pair)
    { return old_id_pair.second == end_node_id; };

    // search for pair with current end_node_id in first position
    auto result =
        std::find_if(boundary_node_assm.begin(), boundary_node_assm.end(), node_in_first_pos);

    // check if first location pair was found
    // if not, re-search in second position
    bool match_first;
    if (result != boundary_node_assm.end())
    {
      match_first = true;
    }
    else
    {
      match_first = false;
      result =
          std::find_if(boundary_node_assm.begin(), boundary_node_assm.end(), node_in_second_pos);
    }

    // check that a pair was found in either first or second position
    if (result != boundary_node_assm.end())
    {
      // based on where the existing node was found
      // add the correct connected node to sorted node list
      boundary_ordered_node_list.push_back(match_first ? (*result).second : (*result).first);
      // remove pair from remaining side list
      boundary_node_assm.erase(result);
    }
    else
    {
      // If there are still elements in boundary_node_assm and result ==
      // boundary_node_assm.end(), this means the boundary is not a loop, the
      // boundary_ordered_node_list is flipped and try the other direction Has not
      // been tested yet.
      if (isFlipped)
      {
        mooseError(
            "The boundary has more than one segments. This code does not work for that case.");
      }

      isFlipped = true;
      std::reverse(boundary_ordered_node_list.begin(), boundary_ordered_node_list.end());
      // As this iteration is wasted, reset the iterator
      i--;
    }
  }
  // If the boundary is a loop, remove the last node as it should be the same as
  // the first one
  if (boundary_ordered_node_list.front() == boundary_ordered_node_list.back())
  {
    boundary_ordered_node_list.erase(boundary_ordered_node_list.end() - 1);
  }

  // resolve node_ids to Node pointers
  for (dof_id_type node_id : boundary_ordered_node_list)
  {
    Node * node = mesh.node_ptr(node_id);
    inner_boundary_nodes.push_back(node);
  }

  return inner_boundary_nodes;
}
