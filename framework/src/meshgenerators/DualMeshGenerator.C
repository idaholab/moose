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
#include "MooseMeshUtils.h"
#include "libmesh/node_elem.h"
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
  return params;
}

DualMeshGenerator::DualMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters), _input(getMesh("input"))
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

  const Real det = A11 * A22 - A12 * A12;

  const Real cx = (A22 * b1 - A12 * b2) / det;
  const Real cy = (A11 * b2 - A12 * b1) / det;

  Point cc(cx, cy, 0.0);

  return cc;
}

// True only when two triangles share a full edge.
static bool
trianglesShareTwoNodes(const Elem * a, const Elem * b)
{
  unsigned int shared_nodes = 0;

  for (unsigned int i = 0; i < a->n_nodes(); ++i)
    for (unsigned int j = 0; j < b->n_nodes(); ++j)
      if (a->node_id(i) == b->node_id(j))
        ++shared_nodes;

  return shared_nodes == 2;
}

/// @brief
/// @return
std::unique_ptr<MeshBase>
DualMeshGenerator::generate()
{
  const auto input_mesh = std::move(_input);

  auto tri_mesh = buildReplicatedMesh(2);

  for (const auto & node : input_mesh->node_ptr_range())
  {
    Node * new_node = tri_mesh->add_point(*node);

    auto node_elem = std::make_unique<NodeElem>();
    node_elem->set_node(0) = new_node;
    tri_mesh->add_elem(std::move(node_elem));
  }

  Node * p0 = tri_mesh->add_point(Point(-100.0, -100.0, 0.0));
  Node * p1 = tri_mesh->add_point(Point(100.0, -100.0, 0.0));
  Node * p2 = tri_mesh->add_point(Point(100.0, 100.0, 0.0));
  Node * p3 = tri_mesh->add_point(Point(-100.0, 100.0, 0.0));

  auto big_square = std::make_unique<Quad4>();

  big_square->set_node(0) = p0;
  big_square->set_node(1) = p1;
  big_square->set_node(2) = p2;
  big_square->set_node(3) = p3;

  if (big_square->is_flipped())
  {
    big_square->set_node(1) = p3;
    big_square->set_node(3) = p1;
  }

  tri_mesh->add_elem(std::move(big_square));

  Poly2TriTriangulator triangulator(dynamic_cast<UnstructuredMesh &>(*tri_mesh));
  triangulator.triangulation_type() = libMesh::TriangulatorInterface::PSLG;
  triangulator.minimum_angle() = 0;
  triangulator.desired_area() = 0;

  triangulator.insert_extra_points() = false;
  triangulator.smooth_after_generating() = false;

  triangulator.triangulate();

  std::vector<Point> circumcenters;
  std::unordered_map<dof_id_type, dof_id_type> tri_elem_to_cc_id;
  std::unordered_map<dof_id_type, std::vector<const Elem *>> primal_node_to_triangles;

  // Compute triangle circumcenters.
  for (const auto & tri_elem : tri_mesh->element_ptr_range())
  {
    if (tri_elem->n_vertices() != 3)
      continue;

    const dof_id_type cc_id = circumcenters.size();

    circumcenters.push_back(circumcenter(tri_elem));
    tri_elem_to_cc_id[tri_elem->id()] = cc_id;

    for (const auto n : make_range(tri_elem->n_nodes()))
      primal_node_to_triangles[tri_elem->node_id(n)].push_back(tri_elem);
  }

  auto dualMesh = buildReplicatedMesh(2);

  // Build one dual element around each primal node.
  for (const auto & [primal_node_id, incident_tris] : primal_node_to_triangles)
  {
    // Connect incident triangles only if they share a full edge.
    std::vector<std::vector<unsigned int>> adjacency(incident_tris.size());

    for (unsigned int i = 0; i < incident_tris.size(); ++i)
      for (unsigned int j = i + 1; j < incident_tris.size(); ++j)
        if (trianglesShareTwoNodes(incident_tris[i], incident_tris[j]))
        {
          adjacency[i].push_back(j);
          adjacency[j].push_back(i);
        }

    // Start at an endpoint for boundary chains, otherwise start anywhere.
    unsigned int start = 0;

    for (unsigned int i = 0; i < adjacency.size(); ++i)
      if (adjacency[i].size() == 1)
      {
        start = i;
        break;
      }

    std::vector<bool> used(incident_tris.size(), false);
    std::vector<dof_id_type> ordered_cc_ids;

    unsigned int current = start;
    unsigned int previous = libMesh::invalid_uint;

    // Walk triangle adjacency to collect circumcenters in connected order.
    while (true)
    {
      const Elem * tri = incident_tris[current];

      auto it = tri_elem_to_cc_id.find(tri->id());
      if (it != tri_elem_to_cc_id.end())
      {
        const dof_id_type cc_id = it->second;

        if (std::find(ordered_cc_ids.begin(), ordered_cc_ids.end(), cc_id) == ordered_cc_ids.end())
          ordered_cc_ids.push_back(cc_id);
      }

      used[current] = true;

      unsigned int next = libMesh::invalid_uint;

      for (const auto candidate : adjacency[current])
        if (candidate != previous && !used[candidate])
        {
          next = candidate;
          break;
        }

      if (next == libMesh::invalid_uint)
        break;

      previous = current;
      current = next;
    }

    if (ordered_cc_ids.size() < 3)
      continue;

    auto dual_elem = std::make_unique<libMesh::C0Polygon>(ordered_cc_ids.size());

    for (unsigned int i = 0; i < ordered_cc_ids.size(); ++i)
      dual_elem->set_node(i, dualMesh->add_point(circumcenters[ordered_cc_ids[i]]));

    if (dual_elem->is_flipped())
    {
      auto reversed_elem = std::make_unique<libMesh::C0Polygon>(ordered_cc_ids.size());

      for (unsigned int i = 0; i < ordered_cc_ids.size(); ++i)
        reversed_elem->set_node(
            i, dualMesh->add_point(circumcenters[ordered_cc_ids[ordered_cc_ids.size() - 1 - i]]));

      dual_elem = std::move(reversed_elem);
    }

    if (!dual_elem->is_flipped())
      dualMesh->add_elem(std::move(dual_elem));
  }

  dualMesh->unset_is_prepared();

  return dynamic_pointer_cast<MeshBase>(dualMesh);
}