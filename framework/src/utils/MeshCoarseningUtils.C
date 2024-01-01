//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCoarseningUtils.h"

namespace MeshCoarseningUtils
{
bool
getFineElementFromInteriorNode(const Node * const interior_node,
                               const Node * const reference_node,
                               const Elem * const elem,
                               const Real /*non_conformality_tol*/,
                               std::vector<const Node *> & tentative_coarse_nodes,
                               std::set<const Elem *> & fine_elements)
{
  mooseAssert(elem, "Should have an elem");
  mooseAssert(interior_node, "Should have an interior node");
  mooseAssert(reference_node, "Should have a reference node");
  const auto elem_type = elem->type();

  // Add point neighbors of interior node to list of potentially refined elements
  // This should be the way to do this, it hits a mysterious "unimplemented virtual"
  // elem->find_point_neighbors(*interior_node, fine_elements);
  // bool found_point_neighbor = false;
  for (const auto neigh : elem->neighbor_ptr_range())
  {
    if (!neigh || neigh == libMesh::remote_elem)
      continue;
    const auto node_index = neigh->get_node_index(interior_node);
    if (node_index != libMesh::invalid_uint && neigh->is_vertex(node_index))
    {
      // Get the neighbor's neighbors, to catch point non-side neighbors
      fine_elements.insert(neigh);
      for (const auto neigh_two : neigh->neighbor_ptr_range())
      {
        if (!neigh_two || neigh_two == libMesh::remote_elem)
          continue;
        const auto node_index_2 = neigh_two->get_node_index(interior_node);
        if (node_index_2 != libMesh::invalid_uint && neigh_two->is_vertex(node_index_2))
        {
          // Get the neighbor's neighbors once
          fine_elements.insert(neigh_two);
        }
      }
    }
  }

  if (elem_type == QUAD4 || elem_type == QUAD8 || elem_type == QUAD9)
  {
    // We need 4 elements around the interior node
    if (fine_elements.size() != 4)
      return false;

    // We need to order the fine elements so when we get the coarse element nodes they form
    // a non-twisted element
    tentative_coarse_nodes.resize(4);

    // The exterior nodes are the opposite nodes of the interior_node!
    unsigned int neighbor_i = 0;
    for (auto neighbor : fine_elements)
    {
      const auto interior_node_number = neighbor->get_node_index(interior_node);
      unsigned int opposite_node_index = (interior_node_number + 2) % 4;

      tentative_coarse_nodes[neighbor_i++] = neighbor->node_ptr(opposite_node_index);
    }

    // Re-order nodes so that they will form a decent quad
    Point axis = (elem->vertex_average() - *interior_node).cross(*interior_node - *reference_node);
    reorderNodes(tentative_coarse_nodes, interior_node, reference_node, axis);
    return true;
  }
  // For hexes we first look at the fine-neighbors of the non-conformality
  // then the fine elements neighbors of the center 'node' of the potential parent
  else
  {
    mooseError("Not implemented for element type " + Moose::stringify(elem_type));
  }
}

void
reorderNodes(std::vector<const Node *> & nodes,
             const Point * origin,
             const Point * clock_start,
             Point & axis)
{
  mooseAssert(axis.norm() != 0, "Invalid rotation axis when ordering nodes");
  mooseAssert(origin != clock_start, "Invalid starting direction when ordering nodes");
  mooseAssert(origin, "Should have an origin");
  mooseAssert(clock_start, "Should have a clock start");

  // We'll need to order the coarse nodes based on the clock-wise order of the elements
  // Define a frame in which to compute the angles of the fine elements centers
  // angle 0 is the [interior node, non-conformal node] vertex
  auto start_clock = *origin - *clock_start;
  start_clock /= start_clock.norm();
  axis /= axis.norm();

  std::vector<std::pair<unsigned int, Real>> nodes_angles(nodes.size());
  for (const auto angle_i : index_range(nodes))
  {
    mooseAssert(nodes[angle_i], "Nodes cant be nullptr");
    auto vec = *nodes[angle_i] - *origin;
    vec /= vec.norm();
    const auto angle = atan2(vec.cross(start_clock) * axis, vec * start_clock);
    nodes_angles[angle_i] = std::make_pair(angle_i, angle);
  }

  // sort by angle, so it goes around the interior node
  std::sort(nodes_angles.begin(),
            nodes_angles.end(),
            [](auto & left, auto & right) { return left.second < right.second; });

  // Re-sort the nodes based on their angle
  std::vector<const Node *> new_nodes(nodes.size());
  for (const auto & old_index : index_range(nodes))
    new_nodes[old_index] = nodes[nodes_angles[old_index].first];
  for (const auto & index : index_range(nodes))
    nodes[index] = new_nodes[index];
}
}
