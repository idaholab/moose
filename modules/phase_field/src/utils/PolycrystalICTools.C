/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PolycrystalICTools.h"
#include "MooseMesh.h"

const unsigned int INVALID_COLOR = std::numeric_limits<unsigned int>::max();

// Forward declarations
bool colorGraph(const std::vector<std::vector<bool> > & adjacency_matrix, std::vector<unsigned int> & colors, unsigned int n_vertices, unsigned int n_ops, unsigned int vertex);
bool isGraphValid(const std::vector<std::vector<bool> > & adjacency_matrix, std::vector<unsigned int> & colors, unsigned int n_vertices,
                  unsigned int vertex, unsigned int color);


std::vector<unsigned>
PolycrystalICTools::assignPointsToVariables(const std::vector<Point> & centerpoints, const Real op_num, const MooseMesh & mesh, const MooseVariable & var)
{
  Real grain_num = centerpoints.size();

  std::vector<unsigned int> assigned_op(grain_num);
  std::vector<int> min_op_ind(op_num);
  std::vector<Real> min_op_dist(op_num);

  //Assign grains to specific order parameters in a way that maximizes the distance
  for (unsigned int grain = 0; grain < grain_num; grain++)
  {
    // Determine the distance to the closest center assigned to each order parameter
    if (grain >= op_num)
    {
      // We can set the array to the distances to the grains 0..op_num-1 (see assignment in the else case)
      for (unsigned int i=0; i<op_num; ++i)
      {
        min_op_dist[i] = mesh.minPeriodicDistance(var.number(), centerpoints[grain], centerpoints[i]);
        min_op_ind[assigned_op[i]] = i;
      }

      // Now check if any of the extra grains are even closer
      for (unsigned int i=op_num; i<grain; ++i)
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
    for (unsigned int i = 1; i < op_num; i++) // Find index of max
      if (min_op_dist[mx_ind] < min_op_dist[i])
        mx_ind = i;

    assigned_op[grain] = mx_ind;
  }

  return assigned_op;
}

unsigned int
PolycrystalICTools::assignPointToGrain(const Point & p, const std::vector<Point> & centerpoints, const MooseMesh & mesh, const MooseVariable & var, const Real maxsize)
{
  unsigned int grain_num = centerpoints.size();

  Real min_distance = maxsize;
  unsigned int min_index = grain_num;
  //Loops through all of the grain centers and finds the center that is closest to the point p
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

std::vector<std::vector<bool> >
PolycrystalICTools::buildGrainAdjacencyGraph(const std::map<dof_id_type, unsigned int> & node_to_grain, MooseMesh & mesh, unsigned int n_grains)
{
  // Build node to elem map
  std::vector<std::vector<const Elem *> > nodes_to_elem_map;
  MeshTools::build_nodes_to_elem_map(mesh.getMesh(), nodes_to_elem_map);

  std::vector<std::vector<bool> > adjacency_matrix(n_grains);
  // initialize
  for (unsigned int i = 0; i < n_grains; ++i)
    adjacency_matrix[i].resize(n_grains, false);

  // TODO: Possibly parallelize this algorithm
  const MeshBase::node_iterator end = mesh.getMesh().active_nodes_end();
  for (MeshBase::node_iterator nl = mesh.getMesh().active_nodes_begin(); nl != end; ++nl)
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
      std::map<dof_id_type, unsigned int>::const_iterator grain_it2 = node_to_grain.find(neighbor_node->id());
      mooseAssert(grain_it2 != node_to_grain.end(), "Node not found in map");
      unsigned int their_grain = grain_it2->second;

      if (my_grain != their_grain)
        adjacency_matrix[my_grain][their_grain] = 1;
    }
  }

  return adjacency_matrix;
}

std::vector<unsigned int>
PolycrystalICTools::assignOpsToGrains(const std::vector<std::vector<bool> > & adjacency_matrix, unsigned int n_grains, unsigned int n_ops)
{
  std::vector<unsigned int> grain_to_op(n_grains, INVALID_COLOR);

  if (!colorGraph(adjacency_matrix, grain_to_op, n_grains, n_ops, 0))
    mooseError("Unable to find a valid Grain to op configuration, do you have enough op variables?");

  return grain_to_op;
}

/**
 * Backtracking graph coloring routines
 */
bool colorGraph(const std::vector<std::vector<bool> > & adjacency_matrix, std::vector<unsigned int> & colors,
                unsigned int n_vertices, unsigned int n_colors, unsigned int vertex)
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

      if (colorGraph(adjacency_matrix, colors, n_vertices, n_colors, vertex+1))
        return true;

      // Backtrack...
      colors[vertex] = INVALID_COLOR;
    }
  }

  return false;
}

bool isGraphValid(const std::vector<std::vector<bool> > & adjacency_matrix, std::vector<unsigned int> & colors, unsigned int n_vertices,
                  unsigned int vertex, unsigned int color)
{
  // See if the proposed color is valid based on the current neighbor colors
  for (unsigned int neighbor = 0; neighbor < n_vertices; ++neighbor)
    if (adjacency_matrix[vertex][neighbor] && color == colors[neighbor])
      return false;
  return true;
}
