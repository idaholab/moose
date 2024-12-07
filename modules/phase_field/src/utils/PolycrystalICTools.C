//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalICTools.h"

// MOOSE includes
#include "MooseMesh.h"
#include "MooseVariable.h"

#include "libmesh/mesh_tools.h"
#include "libmesh/periodic_boundaries.h"
#include "libmesh/point_locator_base.h"

using namespace libMesh;

namespace GraphColoring
{
const unsigned int INVALID_COLOR = std::numeric_limits<unsigned int>::max();
}

namespace PolycrystalICTools
{
const unsigned int HALO_THICKNESS = 4;
}

// Forward declarations
bool colorGraph(const PolycrystalICTools::AdjacencyMatrix<Real> & adjacency_matrix,
                std::vector<unsigned int> & colors,
                unsigned int n_vertices,
                unsigned int n_ops,
                unsigned int vertex);
bool isGraphValid(const PolycrystalICTools::AdjacencyMatrix<Real> & adjacency_matrix,
                  std::vector<unsigned int> & colors,
                  unsigned int n_vertices,
                  unsigned int vertex,
                  unsigned int color);
void visitElementalNeighbors(const Elem * elem,
                             const MeshBase & mesh,
                             const PointLocatorBase & point_locator,
                             const PeriodicBoundaries * pb,
                             std::set<dof_id_type> & halo_ids);

std::vector<unsigned int>
PolycrystalICTools::assignPointsToVariables(const std::vector<Point> & centerpoints,
                                            const Real op_num,
                                            const MooseMesh & mesh,
                                            const MooseVariable & var)
{
  Real grain_num = centerpoints.size();

  std::vector<unsigned int> assigned_op(grain_num);
  std::vector<int> min_op_ind(op_num);
  std::vector<Real> min_op_dist(op_num);

  // Assign grains to specific order parameters in a way that maximizes the distance
  for (unsigned int grain = 0; grain < grain_num; grain++)
  {
    // Determine the distance to the closest center assigned to each order parameter
    if (grain >= op_num)
    {
      // We can set the array to the distances to the grains 0..op_num-1 (see assignment in the else
      // case)
      for (unsigned int i = 0; i < op_num; ++i)
      {
        min_op_dist[i] =
            mesh.minPeriodicDistance(var.number(), centerpoints[grain], centerpoints[i]);
        min_op_ind[assigned_op[i]] = i;
      }

      // Now check if any of the extra grains are even closer
      for (unsigned int i = op_num; i < grain; ++i)
      {
        Real dist = mesh.minPeriodicDistance(var.number(), centerpoints[grain], centerpoints[i]);
        if (min_op_dist[assigned_op[i]] > dist)
        {
          min_op_dist[assigned_op[i]] = dist;
          min_op_ind[assigned_op[i]] = i;
        }
      }
    }
    else
    {
      assigned_op[grain] = grain;
      continue;
    }

    // Assign the current center point to the order parameter that is furthest away.
    unsigned int mx_ind = 0;
    for (unsigned int i = 1; i < op_num; ++i) // Find index of max
      if (min_op_dist[mx_ind] < min_op_dist[i])
        mx_ind = i;

    assigned_op[grain] = mx_ind;
  }

  return assigned_op;
}

unsigned int
PolycrystalICTools::assignPointToGrain(const Point & p,
                                       const std::vector<Point> & centerpoints,
                                       const MooseMesh & mesh,
                                       const MooseVariable & var,
                                       const Real maxsize)
{
  unsigned int grain_num = centerpoints.size();

  Real min_distance = maxsize;
  unsigned int min_index = grain_num;
  // Loops through all of the grain centers and finds the center that is closest to the point p
  for (unsigned int grain = 0; grain < grain_num; grain++)
  {
    Real distance = mesh.minPeriodicDistance(var.number(), centerpoints[grain], p);

    if (min_distance > distance)
    {
      min_distance = distance;
      min_index = grain;
    }
  }

  if (min_index >= grain_num)
    mooseError("ERROR in PolycrystalVoronoiVoidIC: didn't find minimum values in grain_value_calc");

  return min_index;
}

PolycrystalICTools::AdjacencyMatrix<Real>
PolycrystalICTools::buildGrainAdjacencyMatrix(
    const std::map<dof_id_type, unsigned int> & entity_to_grain,
    MooseMesh & mesh,
    const PeriodicBoundaries * pb,
    unsigned int n_grains,
    bool is_elemental)
{
  if (is_elemental)
    return buildElementalGrainAdjacencyMatrix(entity_to_grain, mesh, pb, n_grains);
  else
    return buildNodalGrainAdjacencyMatrix(entity_to_grain, mesh, pb, n_grains);
}

PolycrystalICTools::AdjacencyMatrix<Real>
PolycrystalICTools::buildElementalGrainAdjacencyMatrix(
    const std::map<dof_id_type, unsigned int> & element_to_grain,
    MooseMesh & mesh,
    const PeriodicBoundaries * pb,
    unsigned int n_grains)
{
  AdjacencyMatrix<Real> adjacency_matrix(n_grains);

  // We can't call this in the constructor, it appears that _mesh_ptr is always NULL there.
  mesh.errorIfDistributedMesh("advanced_op_assignment = true");

  std::vector<const Elem *> all_active_neighbors;

  std::vector<std::set<dof_id_type>> local_ids(n_grains);
  std::vector<std::set<dof_id_type>> halo_ids(n_grains);

  std::unique_ptr<PointLocatorBase> point_locator = mesh.getMesh().sub_point_locator();
  for (const auto & elem : mesh.getMesh().active_element_ptr_range())
  {
    std::map<dof_id_type, unsigned int>::const_iterator grain_it =
        element_to_grain.find(elem->id());
    mooseAssert(grain_it != element_to_grain.end(), "Element not found in map");
    unsigned int my_grain = grain_it->second;

    all_active_neighbors.clear();
    // Loop over all neighbors (at the the same level as the current element)
    for (unsigned int i = 0; i < elem->n_neighbors(); ++i)
    {
      const Elem * neighbor_ancestor = elem->topological_neighbor(i, mesh, *point_locator, pb);
      if (neighbor_ancestor)
        // Retrieve only the active neighbors for each side of this element, append them to the list
        // of active neighbors
        neighbor_ancestor->active_family_tree_by_topological_neighbor(
            all_active_neighbors, elem, mesh, *point_locator, pb, false);
    }

    // Loop over all active element neighbors
    for (std::vector<const Elem *>::const_iterator neighbor_it = all_active_neighbors.begin();
         neighbor_it != all_active_neighbors.end();
         ++neighbor_it)
    {
      const Elem * neighbor = *neighbor_it;
      std::map<dof_id_type, unsigned int>::const_iterator grain_it2 =
          element_to_grain.find(neighbor->id());
      mooseAssert(grain_it2 != element_to_grain.end(), "Element not found in map");
      unsigned int their_grain = grain_it2->second;

      if (my_grain != their_grain)
      {
        /**
         * We've found a grain neighbor interface. In order to assign order parameters though, we
         * need to make sure that we build out a small buffer region to avoid literal "corner cases"
         * where nodes on opposite corners of a QUAD end up with the same OP because those nodes are
         * not nodal neighbors. To do that we'll build a halo region based on these interface nodes.
         * For now, we need to record the nodes inside of the grain and those outside of the grain.
         */

        // First add corresponding element and grain information
        local_ids[my_grain].insert(elem->id());
        local_ids[their_grain].insert(neighbor->id());

        // Now add opposing element and grain information
        halo_ids[my_grain].insert(neighbor->id());
        halo_ids[their_grain].insert(elem->id());
      }
      //  adjacency_matrix[my_grain][their_grain] = 1;
    }
  }

  // Build up the halos
  std::set<dof_id_type> set_difference;
  for (unsigned int i = 0; i < n_grains; ++i)
  {
    std::set<dof_id_type> orig_halo_ids(halo_ids[i]);

    for (unsigned int halo_level = 0; halo_level < PolycrystalICTools::HALO_THICKNESS; ++halo_level)
    {
      for (std::set<dof_id_type>::iterator entity_it = orig_halo_ids.begin();
           entity_it != orig_halo_ids.end();
           ++entity_it)
      {
        if (true)
          visitElementalNeighbors(
              mesh.elemPtr(*entity_it), mesh.getMesh(), *point_locator, pb, halo_ids[i]);
        else
          mooseError("Unimplemented");
      }

      set_difference.clear();
      std::set_difference(
          halo_ids[i].begin(),
          halo_ids[i].end(),
          local_ids[i].begin(),
          local_ids[i].end(),
          std::insert_iterator<std::set<dof_id_type>>(set_difference, set_difference.begin()));

      halo_ids[i].swap(set_difference);
    }
  }

  // Finally look at the halo intersections to build the connectivity graph
  std::set<dof_id_type> set_intersection;
  for (unsigned int i = 0; i < n_grains; ++i)
    for (unsigned int j = i + 1; j < n_grains; ++j)
    {
      set_intersection.clear();
      std::set_intersection(
          halo_ids[i].begin(),
          halo_ids[i].end(),
          halo_ids[j].begin(),
          halo_ids[j].end(),
          std::insert_iterator<std::set<dof_id_type>>(set_intersection, set_intersection.begin()));

      if (!set_intersection.empty())
      {
        adjacency_matrix(i, j) = 1.;
        adjacency_matrix(j, i) = 1.;
      }
    }

  return adjacency_matrix;
}

PolycrystalICTools::AdjacencyMatrix<Real>
PolycrystalICTools::buildNodalGrainAdjacencyMatrix(
    const std::map<dof_id_type, unsigned int> & node_to_grain,
    MooseMesh & mesh,
    const PeriodicBoundaries * /*pb*/,
    unsigned int n_grains)
{
  // Build node to elem map
  std::vector<std::vector<const Elem *>> nodes_to_elem_map;
  MeshTools::build_nodes_to_elem_map(mesh.getMesh(), nodes_to_elem_map);

  AdjacencyMatrix<Real> adjacency_matrix(n_grains);

  const auto end = mesh.getMesh().active_nodes_end();
  for (auto nl = mesh.getMesh().active_nodes_begin(); nl != end; ++nl)
  {
    const Node * node = *nl;
    std::map<dof_id_type, unsigned int>::const_iterator grain_it = node_to_grain.find(node->id());
    mooseAssert(grain_it != node_to_grain.end(), "Node not found in map");
    unsigned int my_grain = grain_it->second;

    std::vector<const Node *> nodal_neighbors;
    MeshTools::find_nodal_neighbors(mesh.getMesh(), *node, nodes_to_elem_map, nodal_neighbors);

    // Loop over all nodal neighbors
    for (unsigned int i = 0; i < nodal_neighbors.size(); ++i)
    {
      const Node * neighbor_node = nodal_neighbors[i];
      std::map<dof_id_type, unsigned int>::const_iterator grain_it2 =
          node_to_grain.find(neighbor_node->id());
      mooseAssert(grain_it2 != node_to_grain.end(), "Node not found in map");
      unsigned int their_grain = grain_it2->second;

      if (my_grain != their_grain)
        /**
         * We've found a grain neighbor interface. In order to assign order parameters though, we
         * need to make sure that we build out a small buffer region to avoid literal "corner cases"
         * where nodes on opposite corners of a QUAD end up with the same OP because those nodes are
         * not nodal neighbors. To do that we'll build a halo region based on these interface nodes.
         * For now, we need to record the nodes inside of the grain and those outside of the grain.
         */
        adjacency_matrix(my_grain, their_grain) = 1.;
    }
  }

  return adjacency_matrix;
}

std::vector<unsigned int>
PolycrystalICTools::assignOpsToGrains(AdjacencyMatrix<Real> & adjacency_matrix,
                                      unsigned int n_grains,
                                      unsigned int n_ops,
                                      const MooseEnum & coloring_algorithm)
{
  std::vector<unsigned int> grain_to_op(n_grains, GraphColoring::INVALID_COLOR);

  // Use a simple backtracking coloring algorithm
  if (coloring_algorithm == "bt")
  {
    if (!colorGraph(adjacency_matrix, grain_to_op, n_grains, n_ops, 0))
      mooseError(
          "Unable to find a valid Grain to op configuration, do you have enough op variables?");
  }
  else // PETSc Coloring algorithms
  {
    const std::string & ca_str = coloring_algorithm;
    Real * am_data = adjacency_matrix.rawDataPtr();
    Moose::PetscSupport::colorAdjacencyMatrix(
        am_data, n_grains, n_ops, grain_to_op, ca_str.c_str());
  }

  return grain_to_op;
}

MooseEnum
PolycrystalICTools::coloringAlgorithms()
{
  return MooseEnum("legacy bt jp power greedy", "legacy");
}

std::string
PolycrystalICTools::coloringAlgorithmDescriptions()
{
  return "The grain neighbor graph coloring algorithm to use. \"legacy\" is the original "
         "algorithm "
         "which does not guarantee a valid coloring. \"bt\" is a simple backtracking algorithm "
         "which will produce a valid coloring but has potential exponential run time. The "
         "remaining algorithms require PETSc but are recommended for larger problems (See "
         "MatColoringType)";
}

/**
 * Utility routines
 */
void
visitElementalNeighbors(const Elem * elem,
                        const MeshBase & mesh,
                        const PointLocatorBase & point_locator,
                        const PeriodicBoundaries * pb,
                        std::set<dof_id_type> & halo_ids)
{
  mooseAssert(elem, "Elem is NULL");

  std::vector<const Elem *> all_active_neighbors;

  // Loop over all neighbors (at the the same level as the current element)
  for (unsigned int i = 0; i < elem->n_neighbors(); ++i)
  {
    const Elem * neighbor_ancestor = elem->topological_neighbor(i, mesh, point_locator, pb);
    if (neighbor_ancestor)
      // Retrieve only the active neighbors for each side of this element, append them to the list
      // of active neighbors
      neighbor_ancestor->active_family_tree_by_topological_neighbor(
          all_active_neighbors, elem, mesh, point_locator, pb, false);
  }

  // Loop over all active element neighbors
  for (std::vector<const Elem *>::const_iterator neighbor_it = all_active_neighbors.begin();
       neighbor_it != all_active_neighbors.end();
       ++neighbor_it)
    if (*neighbor_it)
      halo_ids.insert((*neighbor_it)->id());
}

/**
 * Backtracking graph coloring routines
 */
bool
colorGraph(const PolycrystalICTools::AdjacencyMatrix<Real> & adjacency_matrix,
           std::vector<unsigned int> & colors,
           unsigned int n_vertices,
           unsigned int n_colors,
           unsigned int vertex)
{
  // Base case: All grains are assigned
  if (vertex == n_vertices)
    return true;

  // Consider this grain and try different ops
  for (unsigned int color_idx = 0; color_idx < n_colors; ++color_idx)
  {
    // We'll try to spread these colors around a bit rather than
    // packing them all on the first few colors if we have several colors.
    unsigned int color = (vertex + color_idx) % n_colors;

    if (isGraphValid(adjacency_matrix, colors, n_vertices, vertex, color))
    {
      colors[vertex] = color;

      if (colorGraph(adjacency_matrix, colors, n_vertices, n_colors, vertex + 1))
        return true;

      // Backtrack...
      colors[vertex] = GraphColoring::INVALID_COLOR;
    }
  }

  return false;
}

bool
isGraphValid(const PolycrystalICTools::AdjacencyMatrix<Real> & adjacency_matrix,
             std::vector<unsigned int> & colors,
             unsigned int n_vertices,
             unsigned int vertex,
             unsigned int color)
{
  // See if the proposed color is valid based on the current neighbor colors
  for (unsigned int neighbor = 0; neighbor < n_vertices; ++neighbor)
    if (adjacency_matrix(vertex, neighbor) && color == colors[neighbor])
      return false;
  return true;
}
