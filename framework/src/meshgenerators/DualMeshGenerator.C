//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DualMeshGenerator.h"
#include "Conversion.h"
#include "CastUniquePointer.h"
#include "MooseUtils.h"
#include "MooseMeshUtils.h"
#include "libmesh/poly2tri_triangulator.h"
#include "libmesh/elem.h"

registerMooseObject("MooseApp", DualMeshGenerator);

InputParameters
DualMeshGenerator::validParams()
{
  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Takes a 2D mesh as input and returns a Voronoi dual mesh, i.e.,"
                             "changes each input mode into an element and each input element "
                             "into a node located at its circumcenter.");

  params.addParam<Real>("boundary_node_angular_tol",
                        1,
                        "Tolerance (in degrees) for determining colinearity of boundary sides"
                        "when finding input mesh vertices.");
  return params;
}

DualMeshGenerator::DualMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _boundary_node_angular_tol(getParam<Real>("boundary_node_angular_tol"))
{
}

// Circumcenter method
Point
DualMeshGenerator::circumcenter(const Elem * elem)
{
  const unsigned int n = elem->n_vertices();
  libmesh_assert_greater(n, 2);

  const Point & p0 = elem->point(0);

  Real A11 = 0.;
  Real A12 = 0.;
  Real A22 = 0.;

  Real b1 = 0.;
  Real b2 = 0.;

  for (unsigned int i = 1; i < n; ++i)
  {
    const Point & pi = elem->point(i);

    const Real dx = pi(0) - p0(0);
    const Real dy = pi(1) - p0(1);

    const Real rhs = 0.5 * (pi(0) * pi(0) + pi(1) * pi(1) - p0(0) * p0(0) - p0(1) * p0(1));

    A11 += dx * dx;
    A12 += dx * dy;
    A22 += dy * dy;

    b1 += dx * rhs;
    b2 += dy * rhs;
  }

  auto centroid = [&]() -> Point
  {
    Point c;

    for (unsigned int i = 0; i < n; ++i)
      c += elem->point(i);

    c /= n;

    return c;
  };

  const Real det = A11 * A22 - A12 * A12;

  // if (std::abs(det) < 1e-14)
  //   return centroid();

  const Real cx = (A22 * b1 - A12 * b2) / det;
  const Real cy = (A11 * b2 - A12 * b1) / det;

  Point cc(cx, cy, 0.0);

  // Check whether circumcenter is inside polygon using ray casting.
  bool inside = false;

  for (unsigned int i = 0, j = n - 1; i < n; j = i++)
  {
    const Point & pi = elem->point(i);
    const Point & pj = elem->point(j);

    const bool intersects = ((pi(1) > cc(1)) != (pj(1) > cc(1))) &&
                            (cc(0) < (pj(0) - pi(0)) * (cc(1) - pi(1)) / (pj(1) - pi(1)) + pi(0));

    if (intersects)
      inside = !inside;
  }

  auto pointOnSegment = [&](const Point & p, const Point & a, const Point & b) -> bool
  {
    const Real cross = (p(0) - a(0)) * (b(1) - a(1)) - (p(1) - a(1)) * (b(0) - a(0));

    if (std::abs(cross) > 1e-12)
      return false;

    const Real dot = (p(0) - a(0)) * (b(0) - a(0)) + (p(1) - a(1)) * (b(1) - a(1));

    if (dot < -1e-12)
      return false;

    const Real len_sq = (b(0) - a(0)) * (b(0) - a(0)) + (b(1) - a(1)) * (b(1) - a(1));

    if (dot - len_sq > 1e-12)
      return false;

    return true;
  };

  bool on_boundary = false;

  for (unsigned int i = 0; i < n; ++i)
  {
    const Point & a = elem->point(i);
    const Point & b = elem->point((i + 1) % n);

    if (pointOnSegment(cc, a, b))
    {
      on_boundary = true;
      break;
    }
  }

  /*if (!inside && !on_boundary)
    return centroid();
*/
  return cc;
}

/// @brief
/// @return
std::unique_ptr<MeshBase>
DualMeshGenerator::generate()
{
  const auto input_mesh = std::move(_input);

  auto tri_mesh = dynamic_pointer_cast<MeshBase>(input_mesh->clone());

  if (tri_mesh->mesh_dimension() != 2)
    mooseError("DualMeshGenerator currently only supports 2D meshes.");

  Poly2TriTriangulator triangulator(dynamic_cast<UnstructuredMesh &>(*tri_mesh));
  triangulator.triangulation_type() = libMesh::TriangulatorInterface::PSLG;
  triangulator.minimum_angle() = 0;
  triangulator.desired_area() = 0;

  triangulator.insert_extra_points() = false;
  triangulator.smooth_after_generating() = false;

  triangulator.triangulate();

  std::vector<Point> circumcenters;
  std::unordered_map<dof_id_type, std::vector<dof_id_type>> node_to_circumcenter_ids;
  std::unordered_map<dof_id_type, std::vector<Point>> node_to_boundary_midpoints;
  std::unordered_map<dof_id_type, std::vector<dof_id_type>> node_to_boundary_neighbors;
  std::unordered_set<dof_id_type> boundary_node_ids;
  std::unordered_set<dof_id_type> vertex_node_ids;
  std::unordered_set<dof_id_type> midpoint_node_ids;

  for (const auto & tri_elem : tri_mesh->element_ptr_range())
  {
    if (tri_elem->n_vertices() != 3)
      continue;

    const dof_id_type circumcenter_id = circumcenters.size();
    circumcenters.push_back(circumcenter(tri_elem));

    for (const auto n : make_range(tri_elem->n_nodes()))
      node_to_circumcenter_ids[tri_elem->node_id(n)].push_back(circumcenter_id);

    for (const auto side : tri_elem->side_index_range())
    {
      if (tri_elem->neighbor_ptr(side) == nullptr)
      {
        std::unique_ptr<const Elem> side_elem = tri_elem->build_side_ptr(side);

        if (side_elem->n_nodes() == 2)
        {
          const Node * n0 = side_elem->node_ptr(0);
          const Node * n1 = side_elem->node_ptr(1);

          Point midpoint = 0.5 * ((*n0) + (*n1));

          boundary_node_ids.insert(n0->id());
          boundary_node_ids.insert(n1->id());

          node_to_boundary_midpoints[n0->id()].push_back(midpoint);
          node_to_boundary_midpoints[n1->id()].push_back(midpoint);
          node_to_boundary_neighbors[n0->id()].push_back(n1->id());
          node_to_boundary_neighbors[n1->id()].push_back(n0->id());
        }
      }
    }
  }

  auto isBoundaryVertex = [&](dof_id_type node_id) -> bool
  {
    auto it = node_to_boundary_neighbors.find(node_id);
    if (it == node_to_boundary_neighbors.end())
      return false;

    auto neighbors = it->second;

    std::sort(neighbors.begin(), neighbors.end());
    neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());

    if (neighbors.size() != 2)
      return true;

    const Point & p = *tri_mesh->node_ptr(node_id);

    Point v0 = *tri_mesh->node_ptr(neighbors[0]) - p;
    Point v1 = *tri_mesh->node_ptr(neighbors[1]) - p;

    const Real n0 = v0.norm();
    const Real n1 = v1.norm();

    if (n0 == 0.0 || n1 == 0.0)
      return false;

    Real c = (v0 * v1) / (n0 * n1);
    c = std::max(Real(-1), std::min(Real(1), c));

    const Real angle_deg = std::acos(c) * 180.0 / libMesh::pi;

    return std::abs(angle_deg - 180.0) > _boundary_node_angular_tol;
  };

  auto dualMesh = buildReplicatedMesh(2);

  for (const auto i : index_range(circumcenters))
    dualMesh->add_point(circumcenters[i]);

  for (const auto & [primalNodeID, circumcenterIDs] : node_to_circumcenter_ids)
  {
    const bool is_boundary_node = boundary_node_ids.count(primalNodeID);
    const bool is_boundary_vertex = is_boundary_node && isBoundaryVertex(primalNodeID);

    if (!is_boundary_node && circumcenterIDs.size() < 3)
      continue;

    std::vector<std::pair<Node *, Real>> nodes_and_phis;

    const Point & primal_point = *tri_mesh->node_ptr(primalNodeID);

    if (is_boundary_node)
    {
      if (is_boundary_vertex)
      {
        Node * primal_vertex_node = dualMesh->add_point(*tri_mesh->node_ptr(primalNodeID));
        vertex_node_ids.insert(primal_vertex_node->id());
        nodes_and_phis.push_back({primal_vertex_node, 0.0});
      }

      for (const auto & midpoint : node_to_boundary_midpoints[primalNodeID])
      {
        Node * midpoint_node = dualMesh->add_point(midpoint);
        midpoint_node_ids.insert(midpoint_node->id());
        nodes_and_phis.push_back({midpoint_node, 0.0});
      }
    }

    for (const auto circumcenter_id : circumcenterIDs)
    {
      Node * circumcenter_node = dualMesh->node_ptr(circumcenter_id);
      nodes_and_phis.push_back({circumcenter_node, 0.0});
    }

    Point dual_elem_centroid;

    for (const auto & [node, phi] : nodes_and_phis)
      dual_elem_centroid += *node;

    dual_elem_centroid /= nodes_and_phis.size();

    for (auto & [node, phi] : nodes_and_phis)
    {
      phi = std::atan2((*node)(1) - dual_elem_centroid(1), (*node)(0) - dual_elem_centroid(0));
    }

    std::sort(nodes_and_phis.begin(),
              nodes_and_phis.end(),
              [](const auto & a, const auto & b) { return a.second < b.second; });

    auto isVertexNode = [&](const Node * node) -> bool
    { return vertex_node_ids.find(node->id()) != vertex_node_ids.end(); };

    auto isMidpointNode = [&](const Node * node) -> bool
    { return midpoint_node_ids.find(node->id()) != midpoint_node_ids.end(); };

    Node * vertex_node = nullptr;

    for (const auto & [node, phi] : nodes_and_phis)
    {
      if (isVertexNode(node))
      {
        vertex_node = node;
        break;
      }
    }

    if (vertex_node)
    {
      std::vector<std::pair<Node *, Real>> sorted_nodes;

      for (const auto & [node, phi] : nodes_and_phis)
      {
        if (node != vertex_node)
          sorted_nodes.push_back({node, phi});
      }

      std::vector<Node *> fan_nodes;

      for (unsigned int i = 0; i < sorted_nodes.size(); ++i)
      {
        if (!isMidpointNode(sorted_nodes[i].first))
          continue;

        const unsigned int next_i = (i + 1) % sorted_nodes.size();
        const unsigned int prev_i = (i + sorted_nodes.size() - 1) % sorted_nodes.size();

        if (!isMidpointNode(sorted_nodes[next_i].first))
        {
          for (unsigned int k = 0; k < sorted_nodes.size(); ++k)
            fan_nodes.push_back(sorted_nodes[(i + k) % sorted_nodes.size()].first);

          break;
        }

        if (!isMidpointNode(sorted_nodes[prev_i].first))
        {
          for (unsigned int k = 0; k < sorted_nodes.size(); ++k)
            fan_nodes.push_back(
                sorted_nodes[(i + sorted_nodes.size() - k) % sorted_nodes.size()].first);

          break;
        }
      }

      if (fan_nodes.empty())
        mooseError("Could not find a boundary midpoint to start boundary fan triangulation.");

      for (unsigned int i = 0; i + 1 < fan_nodes.size(); ++i)
      {
        auto tri = std::make_unique<libMesh::C0Polygon>(3);
        tri->set_node(0, vertex_node);
        tri->set_node(1, fan_nodes[i]);
        tri->set_node(2, fan_nodes[i + 1]);

        if (tri->is_flipped())
        {
          tri->set_node(1, fan_nodes[i + 1]);
          tri->set_node(2, fan_nodes[i]);
        }

        libmesh_assert(!tri->is_flipped());
        dualMesh->add_elem(std::move(tri));
      }
    }
    else
    {
      auto dual_elem = std::make_unique<libMesh::C0Polygon>(nodes_and_phis.size());

      for (unsigned int i = 0; i < nodes_and_phis.size(); ++i)
        dual_elem->set_node(i, nodes_and_phis[i].first);
      libmesh_assert(!dual_elem->is_flipped());
      dualMesh->add_elem(std::move(dual_elem));
    }
  }

  dualMesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(dualMesh);
  //  return dynamic_pointer_cast<MeshBase>(tri_mesh);
}
