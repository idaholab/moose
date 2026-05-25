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

#include "libmesh/elem.h"

registerMooseObject("MooseApp", DualMeshGenerator);

InputParameters
DualMeshGenerator::validParams()
{
  MooseEnum location("INSIDE OUTSIDE", "INSIDE");

  InputParameters params = MeshGenerator::validParams();

  params.addRequiredParam<MeshGeneratorName>("input", "The mesh we want to modify");
  params.addClassDescription("Takes a 2D mesh as input and returns a Voronoi dual mesh, i.e.,"
                             "changes each input mode into an element and each input element "
                             "into a node located at its centroid.");

  params.addParam<std::string>("integer_name",
                               "Element integer to be assigned (default to subdomain ID)");
  params.addParam<Real>("boundary_node_angular_tol",
                        1,
                        "Tolerance (in degrees) for determining colinearity of boundary sides"
                        "when finding input mesh vertices. Default: 1");
  return params;
}

DualMeshGenerator::DualMeshGenerator(const InputParameters & parameters)
  : MeshGenerator(parameters),
    _input(getMesh("input")),
    _boundary_node_angular_tol(getParam<Real>("boundary_node_angular_tol"))

{
}

std::unique_ptr<MeshBase>
DualMeshGenerator::generate()
{
  const auto mesh = std::move(_input);
  std::vector<libMesh::Point> centroids; // vector of all dual nodes
  centroids.reserve(mesh->n_elem());

  std::unordered_map<dof_id_type, std::vector<dof_id_type>> _node_to_elem_map;

  unsigned int i;

  //////////Adding nodes//////
  for (const auto & in_elem : mesh->element_ptr_range())
  {

    _console << "Found centroid: " << in_elem->true_centroid() << std::endl;

    Point centroid = in_elem->true_centroid();
    centroids.push_back(centroid);
    for (unsigned int n = 0; n < in_elem->n_nodes(); n++)
    {
      _node_to_elem_map[in_elem->node_id(n)].push_back(in_elem->id());
    }
  }

  auto dualMesh = buildReplicatedMesh(mesh->mesh_dimension());

  for (i = 0; i < centroids.size(); ++i)
  {
    dualMesh->add_point(centroids[i]);
  }

  std::unordered_map<dof_id_type, std::vector<dof_id_type>> _elem_to_node_map;
  for (const auto & [node_id, elements] : _node_to_elem_map)
  {
    for (dof_id_type elem_id : elements)
    {
      _elem_to_node_map[elem_id].push_back(node_id);
    }
  } //_elem_to_node_map now has element IDs in the first entry and maps to the nodes that make
    // up that element

  _console << "Mesh populated with dual nodes" << std::endl;

  // Get what nodes are boundary nodes
  // looping over primal elements....
  std::unordered_map<dof_id_type, std::vector<Point>> node_to_boundary_midpoints;
  std::unordered_map<dof_id_type, std::vector<dof_id_type>> node_to_boundary_neighbors;
  std::unordered_set<dof_id_type> corner_node_ids;
  std::unordered_set<dof_id_type> midpoint_node_ids;
  for (const auto & primalElem : mesh->element_ptr_range())
  {
    // looping over each side
    for (const auto & side : primalElem->side_index_range())
    {
      if (primalElem->neighbor_ptr(side) == nullptr) // if its a boundary side..
      {
        std::unique_ptr<Elem> side_elem = primalElem->build_side_ptr(side);

        std::vector<Node *> side_nodes;
        _console << "found bordering side with nodes: " << std::endl;
        for (unsigned int i = 0; i < side_elem->n_nodes(); ++i)
        {
          Node * node = side_elem->node_ptr(i);
          node->print_info();
          side_nodes.push_back(node);
        }

        // find midpoint.
        Point midPoint;
        for (const auto & node : side_nodes)
        {
          midPoint += *node;
        }
        midPoint /= side_nodes.size();

        for (auto * node : side_nodes)
          node_to_boundary_midpoints[node->id()].push_back(midPoint);

        // Recording boundary edge connectivity
        if (side_nodes.size() == 2)
        {
          const auto id0 = side_nodes[0]->id();
          const auto id1 = side_nodes[1]->id();

          node_to_boundary_neighbors[id0].push_back(id1);
          node_to_boundary_neighbors[id1].push_back(id0);
        }
      }
    }
  }
  // boundaryMidPoints now contains all of the node pointers that are boundary nodes that might need
  // to be added to a dual mesh.
  // Boundary Helper

  auto isBoundaryVertex = [&](dof_id_type node_id) -> bool
  {
    auto it = node_to_boundary_neighbors.find(node_id);
    if (it == node_to_boundary_neighbors.end())
      return false;

    auto neighbors = it->second;

    std::sort(neighbors.begin(), neighbors.end());
    neighbors.erase(std::unique(neighbors.begin(), neighbors.end()), neighbors.end());

    if (neighbors.size() != 2)
      return false;

    const Point & p = *mesh->node_ptr(node_id);
    Point v0 = *mesh->node_ptr(neighbors[0]) - p;
    Point v1 = *mesh->node_ptr(neighbors[1]) - p;

    const Real n0 = v0.norm();
    const Real n1 = v1.norm();

    if (n0 == 0.0 || n1 == 0.0)
      return false;

    Real c = (v0 * v1) / (n0 * n1);
    c = std::max(Real(-1), std::min(Real(1), c));

    const Real angle_deg = std::acos(c) * 180.0 / libMesh::pi;

    return std::abs(angle_deg - 180.0) > _boundary_node_angular_tol;
  };

  auto signedArea = [](const std::vector<Node *> & nodes) -> Real
  {
    Real area = 0.0;

    for (unsigned int i = 0; i < nodes.size(); ++i)
    {
      const Point & p = *nodes[i];
      const Point & q = *nodes[(i + 1) % nodes.size()];

      area += p(0) * q(1) - q(0) * p(1);
    }

    return 0.5 * area;
  };

  auto cross2D = [](const Point & a, const Point & b, const Point & c) -> Real
  { return (b(0) - a(0)) * (c(1) - a(1)) - (b(1) - a(1)) * (c(0) - a(0)); };

  auto isConcavePolygon = [&](const std::vector<Node *> & nodes) -> bool
  {
    if (nodes.size() < 4)
      return false;

    const Real orientation = signedArea(nodes);

    for (unsigned int i = 0; i < nodes.size(); ++i)
    {
      const Point & prev = *nodes[(i + nodes.size() - 1) % nodes.size()];
      const Point & curr = *nodes[i];
      const Point & next = *nodes[(i + 1) % nodes.size()];

      const Real cross = cross2D(prev, curr, next);

      if (orientation > 0.0 && cross < -TOLERANCE)
        return true;

      if (orientation < 0.0 && cross > TOLERANCE)
        return true;
    }

    return false;
  };

  auto addTriangle = [&](Node * a, Node * b, Node * c)
  {
    auto tri = std::make_unique<libMesh::C0Polygon>(3);
    tri->set_node(0, a);
    tri->set_node(1, b);
    tri->set_node(2, c);
    dualMesh->add_elem(std::move(tri));
  };

  auto triangleArea = [](const Node * a, const Node * b, const Node * c) -> Real
  {
    return std::abs(((*b)(0) - (*a)(0)) * ((*c)(1) - (*a)(1)) -
                    ((*b)(1) - (*a)(1)) * ((*c)(0) - (*a)(0))) *
           0.5;
  };

  auto isCornerNode = [&](const Node * node) -> bool
  { return corner_node_ids.find(node->id()) != corner_node_ids.end(); };

  auto isMidpointNode = [&](const Node * node) -> bool
  { return midpoint_node_ids.find(node->id()) != midpoint_node_ids.end(); };

  auto addConvexOrTriangulatedPolygon = [&](std::vector<Node *> nodes)
  {
    if (nodes.size() < 3)
      return;

    if (!isConcavePolygon(nodes))
    {
      auto elem = std::make_unique<libMesh::C0Polygon>(nodes.size());

      for (unsigned int i = 0; i < nodes.size(); ++i)
        elem->set_node(i, nodes[i]);

      dualMesh->add_elem(std::move(elem));
      return;
    }

    Node * cornerNode = nullptr;

    for (auto * node : nodes)
    {
      if (isCornerNode(node))
      {
        cornerNode = node;
        break;
      }
    }

    if (!cornerNode)
      mooseError("Concave polygon was detected, but no corner node was found.");

    std::vector<std::pair<Node *, Real>> sorted_nodes;

    for (auto * node : nodes)
    {
      if (node == cornerNode)
        continue;

      const Real phi = std::atan2((*node)(1) - (*cornerNode)(1), (*node)(0) - (*cornerNode)(0));

      sorted_nodes.push_back({node, phi});
    }

    std::sort(sorted_nodes.begin(),
              sorted_nodes.end(),
              [](const auto & a, const auto & b) { return a.second < b.second; });
    std::vector<Node *> fan_nodes;

    for (unsigned int i = 0; i < sorted_nodes.size(); ++i)
    {
      if (!isMidpointNode(sorted_nodes[i].first))
        continue;

      const unsigned int next_i = (i + 1) % sorted_nodes.size();
      const unsigned int prev_i = (i + sorted_nodes.size() - 1) % sorted_nodes.size();

      // Prefer the direction where the first step from the boundary midpoint
      // goes to an interior/centroid node, not another boundary midpoint.
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
      mooseError("Could not find a boundary midpoint to start concave fan triangulation.");

    for (unsigned int i = 0; i + 1 < fan_nodes.size(); ++i)
      addTriangle(cornerNode, fan_nodes[i], fan_nodes[i + 1]);
  };

  // loop over all primal nodes / dual elements
  for (const auto & [primalNodeID, primalElemIDs] : _node_to_elem_map)
  {
    _console << "Number of nodes for dual element: " << primalElemIDs.size() << std::endl;
    const bool is_boundary_node =
        node_to_boundary_midpoints.find(primalNodeID) != node_to_boundary_midpoints.end();
    std::vector<std::pair<Node *, Real>> dualNodesAndPhis;
    if (!is_boundary_node) // For interior polygons we needn't worry about the boundary
    {
      // Now loop over the # of nodes on each dual element
      auto primalNode = mesh->node_ptr(primalNodeID);

      for (unsigned int j = 0; j < primalElemIDs.size(); ++j)
      {
        const dof_id_type dualNodeOnPElem_id =
            primalElemIDs[j]; // Grab the dual nodes' IDs on each primal element

        auto const dualNodeOnPElem = dualMesh->node_ptr(
            dualNodeOnPElem_id); // Grab the dual nodes corresponding to these IDs

        Real dualNodeX = (dualNodeOnPElem->operator()(0)) - primalNode->operator()(0);
        Real dualNodeY = (dualNodeOnPElem->operator()(1)) - primalNode->operator()(1);
        Real nodePhi = atan2(dualNodeY, dualNodeX);

        dualNodesAndPhis.push_back({dualNodeOnPElem, nodePhi});

        std::sort(dualNodesAndPhis.begin(),
                  dualNodesAndPhis.end(),
                  [](const auto & a, const auto & b) { return a.second < b.second; });
      }
    }
    else // boundary dual elements
    {

      Node * primalNode = mesh->node_ptr(primalNodeID);

      if (isBoundaryVertex(primalNodeID))
      {
        Node * cornerNode = dualMesh->add_point(*mesh->node_ptr(primalNodeID));
        corner_node_ids.insert(cornerNode->id());

        dualNodesAndPhis.push_back({cornerNode, 0.0});
      }

      // Add centroid nodes from adjacent primal elements
      for (const auto elem_id : primalElemIDs)
      {
        Node * dualNode = dualMesh->node_ptr(elem_id);
        dualNodesAndPhis.push_back({dualNode, 0.0});
      }

      // Add boundary midpoint nodes
      for (const auto & midpoint : node_to_boundary_midpoints[primalNodeID])
      {
        Node * midpointNode = dualMesh->add_point(midpoint);
        midpoint_node_ids.insert(midpointNode->id());
        dualNodesAndPhis.push_back({midpointNode, 0.0});
      }

      // Recompute angles around the geometric center of boundary elements

      Point centroid_avg;

      for (const auto elem_id : primalElemIDs)
        centroid_avg += *dualMesh->node_ptr(elem_id);

      centroid_avg /= primalElemIDs.size();

      Point sort_center = 0.5 * ((*primalNode) + centroid_avg);

      for (auto & [node, phi] : dualNodesAndPhis)
      {
        phi = std::atan2((*node)(1) - sort_center(1), (*node)(0) - sort_center(0));
      }
    }
    std::sort(dualNodesAndPhis.begin(),
              dualNodesAndPhis.end(),
              [](const auto & a, const auto & b) { return a.second < b.second; });

    std::vector<Node *> ordered_nodes;
    ordered_nodes.reserve(dualNodesAndPhis.size());

    for (const auto & node_phi : dualNodesAndPhis)
      ordered_nodes.push_back(node_phi.first);

    addConvexOrTriangulatedPolygon(ordered_nodes);
  }

  dualMesh->unset_is_prepared();
  return dynamic_pointer_cast<MeshBase>(dualMesh);
}