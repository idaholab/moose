//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MeshCoarseningUtils.h"
#include "Conversion.h"
#include "MooseError.h"

#include "libmesh/enum_elem_type.h"
#include "libmesh/remote_elem.h"

using namespace libMesh;

namespace MeshCoarseningUtils
{
bool
getFineElementsFromInteriorNode(const libMesh::Node & interior_node,
                                const libMesh::Node & reference_node,
                                const libMesh::Elem & fine_elem,
                                std::vector<const libMesh::Node *> & tentative_coarse_nodes,
                                std::set<const libMesh::Elem *> & fine_elements)
{
  const auto elem_type = fine_elem.type();

  // Add point neighbors of interior node to list of potentially refined elements
  // NOTE: we could potentially replace this with a simple call to point_neighbors
  // on a fine element with the interior node. It's not clear which approach is more
  // resilient to meshes with slits from discarded adaptivity information
  fine_elements.insert(&fine_elem);
  for (const auto neigh : fine_elem.neighbor_ptr_range())
  {
    if (!neigh || neigh == libMesh::remote_elem)
      continue;
    const auto node_index = neigh->get_node_index(&interior_node);
    if (node_index != libMesh::invalid_uint && neigh->is_vertex(node_index))
    {
      // Get the neighbor's neighbors, to catch point non-side neighbors
      // This is needed in 2D to get all quad neighbors
      fine_elements.insert(neigh);
      for (const auto neigh_two : neigh->neighbor_ptr_range())
      {
        if (!neigh_two || neigh_two == libMesh::remote_elem)
          continue;
        const auto node_index_2 = neigh_two->get_node_index(&interior_node);
        if (node_index_2 != libMesh::invalid_uint && neigh_two->is_vertex(node_index_2))
        {
          // Get the neighbor's neighbors
          fine_elements.insert(neigh_two);

          // Get the neighbor's neighbors' neighbors, to catch point non-side neighbors
          // This is needed for 3D to get all hex neighbors
          for (const auto neigh_three : neigh_two->neighbor_ptr_range())
          {
            if (!neigh_three || neigh_three == libMesh::remote_elem)
              continue;
            const auto node_index_3 = neigh_three->get_node_index(&interior_node);
            if (node_index_3 != libMesh::invalid_uint && neigh_three->is_vertex(node_index_3))
              fine_elements.insert(neigh_three);
          }
        }
      }
    }
  }

  // If the fine elements are not all of the same type, we do not know how to get the opposite node
  // of the interior node in the fine elements
  for (auto elem : fine_elements)
    if (elem && fine_elem.type() != elem_type)
      return false;

  if (elem_type == libMesh::QUAD4 || elem_type == libMesh::QUAD8 || elem_type == libMesh::QUAD9)
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
      const auto interior_node_number = neighbor->get_node_index(&interior_node);
      unsigned int opposite_node_index = (interior_node_number + 2) % 4;

      tentative_coarse_nodes[neighbor_i++] = neighbor->node_ptr(opposite_node_index);
    }

    // Re-order nodes so that they will form a decent quad
    libMesh::Point axis =
        (fine_elem.vertex_average() - interior_node).cross(interior_node - reference_node);
    reorderNodes(tentative_coarse_nodes, interior_node, reference_node, axis);
    return true;
  }
  // For hexes, similar strategy but we need to pick 4 nodes to form a side, then 4 other nodes
  // facing those initial nodes
  else if (elem_type == libMesh::HEX8)
  {
    // We need 8 elements around the interior node
    if (fine_elements.size() != 8)
      return false;

    tentative_coarse_nodes.resize(4);

    // Pick a node (mid-face for the coarse element) to form the base
    // We must use the same element reproducibly, despite the pointers being ordered in the set
    // We use the element id to choose the same element consistently
    const Elem * one_fine_elem = nullptr;
    unsigned int max_id = 0;
    for (const auto elem_ptr : fine_elements)
      if (elem_ptr->id() > max_id)
      {
        max_id = elem_ptr->id();
        one_fine_elem = elem_ptr;
      }
    const auto interior_node_index = one_fine_elem->get_node_index(&interior_node);

    // Find any side which contains the interior node
    unsigned int an_interior_node_side = 0;
    for (const auto s : make_range(one_fine_elem->n_sides()))
      if (one_fine_elem->is_node_on_side(interior_node_index, s))
      {
        an_interior_node_side = s;
        break;
      }
    // A node near a face of the coarse element seek is on the same side, but opposite from the
    // interior node
    const auto center_face_node_index =
        one_fine_elem->opposite_node(interior_node_index, an_interior_node_side);
    const auto center_face_node = one_fine_elem->node_ptr(center_face_node_index);

    // We gather the coarse element nodes from the fine elements that share the center face node we
    // just selected
    unsigned int neighbor_i = 0;
    std::vector<const libMesh::Elem *> other_fine_elems;
    for (auto neighbor : fine_elements)
    {
      if (neighbor->get_node_index(center_face_node) == libMesh::invalid_uint)
      {
        other_fine_elems.push_back(neighbor);
        continue;
      }
      // The coarse element node is the opposite nodes of the interior_node in a fine element
      const auto interior_node_number = neighbor->get_node_index(&interior_node);
      unsigned int opposite_node_index =
          getOppositeNodeIndex(neighbor->type(), interior_node_number);

      tentative_coarse_nodes[neighbor_i++] = neighbor->node_ptr(opposite_node_index);
    }

    // Center face node was not shared with 4 elements
    // We could try again on any of the 5 other coarse element faces but we don't insist for now
    if (neighbor_i != 4 || other_fine_elems.size() != 4)
      return false;

    // Sort the coarse nodes so we are reproducibly picking the same ordering of nodes
    auto cmp_node = [](const Node * a, const Node * b) { return a->id() < b->id(); };
    std::sort(tentative_coarse_nodes.begin(), tentative_coarse_nodes.end(), cmp_node);

    // Re-order nodes so that they will form a decent quad
    // Pick the reference node for the rotation frame as the face center
    const libMesh::Point clock_start = *tentative_coarse_nodes[0];
    libMesh::Point axis = interior_node - *center_face_node;
    reorderNodes(tentative_coarse_nodes, *center_face_node, clock_start, axis);

    // Look through the 4 other fine elements to finish the coarse hex element nodes
    for (const auto coarse_node_index : make_range(4))
    {
      // Find the fine element containing each coarse node already found & ordered
      const Elem * fine_elem = nullptr;
      for (auto elem : fine_elements)
        if (elem->get_node_index(tentative_coarse_nodes[coarse_node_index]) !=
            libMesh::invalid_uint)
        {
          fine_elem = elem;
          break;
        }
      mooseAssert(fine_elem, "Search for fine element should have worked");

      // Find the other fine element opposite the element containing the node (they share a side)
      const Elem * fine_neighbor = nullptr;
      for (auto neighbor : other_fine_elems)
        // Side neighbor. This requires the mesh to have correct neighbors
        // For meshes with lost AMR information, this wont work
        if (neighbor->which_neighbor_am_i(fine_elem) != libMesh::invalid_uint)
        {
          if (fine_neighbor)
            mooseError("Found two neighbors");
          fine_neighbor = neighbor;
        }
      // the fine element in the base of the coarse hex is not a neighbor to any element
      // in the top part. The mesh is probably slit in the middle of the potential coarse hex
      // element. We wont support this for now.
      if (!fine_neighbor)
        return false;

      // Get the coarse node, opposite the interior node in that fine element
      const auto interior_node_index_neighbor = fine_neighbor->get_node_index(&interior_node);
      tentative_coarse_nodes.push_back(fine_neighbor->node_ptr(
          getOppositeNodeIndex(fine_neighbor->type(), interior_node_index_neighbor)));
    }

    // Found 8 fine elements and 8 coarse element nodes as expected
    if (tentative_coarse_nodes.size() == 8)
      return true;
    else
      return false;
  }
  else
    mooseError("Not implemented for element type " + Moose::stringify(elem_type));
}

void
reorderNodes(std::vector<const libMesh::Node *> & nodes,
             const libMesh::Point & origin,
             const libMesh::Point & clock_start,
             libMesh::Point & axis)
{
  mooseAssert(axis.norm() != 0, "Invalid rotation axis when ordering nodes");
  mooseAssert(origin != clock_start, "Invalid starting direction when ordering nodes");

  // We'll need to order the coarse nodes based on the clock-wise order of the elements
  // Define a frame in which to compute the angles of the fine elements centers
  // angle 0 is the [interior node, non-conformal node] vertex
  auto start_clock = origin - clock_start;
  start_clock /= start_clock.norm();
  axis /= axis.norm();

  std::vector<std::pair<unsigned int, libMesh::Real>> nodes_angles(nodes.size());
  for (const auto angle_i : index_range(nodes))
  {
    mooseAssert(nodes[angle_i], "Nodes cant be nullptr");
    auto vec = *nodes[angle_i] - origin;
    vec /= vec.norm();
    const auto angle = atan2(vec.cross(start_clock) * axis, vec * start_clock);
    nodes_angles[angle_i] = std::make_pair(angle_i, angle);
  }

  // sort by angle, so it goes around the interior node
  std::sort(nodes_angles.begin(),
            nodes_angles.end(),
            [](auto & left, auto & right) { return left.second < right.second; });

  // Re-sort the nodes based on their angle
  std::vector<const libMesh::Node *> new_nodes(nodes.size());
  for (const auto & old_index : index_range(nodes))
    new_nodes[old_index] = nodes[nodes_angles[old_index].first];
  for (const auto & index : index_range(nodes))
    nodes[index] = new_nodes[index];
}

unsigned int
getOppositeNodeIndex(libMesh::ElemType elem_type, unsigned int node_index)
{
  switch (elem_type)
  {
    case QUAD4:
      return (node_index + 2) % 4;
    case HEX8:
    {
      mooseAssert(node_index < 8, "Node index too high: " + std::to_string(node_index));
      return std::vector<unsigned int>({6, 7, 4, 5, 2, 3, 0, 1})[node_index];
    }
    default:
      mooseError("Unsupported element type for retrieving the opposite node");
  }
}
}
