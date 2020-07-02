//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NearestNodeThread.h"
#include "MooseMesh.h"

#include "libmesh/threads.h"
#include "libmesh/node.h"

#include <cmath>

NearestNodeThread::NearestNodeThread(
    const MooseMesh & mesh, std::map<dof_id_type, std::vector<dof_id_type>> & neighbor_nodes)
  : _max_patch_percentage(0.0), _mesh(mesh), _neighbor_nodes(neighbor_nodes)
{
}

// Splitting Constructor
NearestNodeThread::NearestNodeThread(NearestNodeThread & x, Threads::split /*split*/)
  : _max_patch_percentage(x._max_patch_percentage),
    _mesh(x._mesh),
    _neighbor_nodes(x._neighbor_nodes)
{
}

/**
 * Save a patch of nodes that are close to each of the secondary nodes to speed the search algorithm
 * TODO: This needs to be updated at some point in time.  If the hits into this data structure
 * approach "the end"
 * then it may be time to update
 */
void
NearestNodeThread::operator()(const NodeIdRange & range)
{
  for (const auto & node_id : range)
  {
    const Node & node = _mesh.nodeRef(node_id);

    const Node * closest_node = NULL;
    Real closest_distance = std::numeric_limits<Real>::max();

    const std::vector<dof_id_type> & neighbor_nodes = _neighbor_nodes[node_id];

    unsigned int n_neighbor_nodes = neighbor_nodes.size();

    for (unsigned int k = 0; k < n_neighbor_nodes; k++)
    {
      const Node * cur_node = &_mesh.nodeRef(neighbor_nodes[k]);
      Real distance = ((*cur_node) - node).norm();

      if (distance < closest_distance)
      {
        Real patch_percentage = static_cast<Real>(k) / static_cast<Real>(n_neighbor_nodes);

        // Save off the maximum we had to go through the patch to find the closes node
        if (patch_percentage > _max_patch_percentage)
          _max_patch_percentage = patch_percentage;

        closest_distance = distance;
        closest_node = cur_node;
      }
    }

    if (closest_distance == std::numeric_limits<Real>::max())
    {
      for (unsigned int k = 0; k < n_neighbor_nodes; k++)
      {
        const Node * cur_node = &_mesh.nodeRef(neighbor_nodes[k]);
        if (std::isnan((*cur_node)(0)) || std::isinf((*cur_node)(0)) ||
            std::isnan((*cur_node)(1)) || std::isinf((*cur_node)(1)) ||
            std::isnan((*cur_node)(2)) || std::isinf((*cur_node)(2)))
          throw MooseException(
              "Failure in NearestNodeThread because solution contains inf or not-a-number "
              "entries.  This is likely due to a failed factorization of the Jacobian "
              "matrix.");
      }
      mooseError("Unable to find nearest node!");
    }

    NearestNodeLocator::NearestNodeInfo & info = _nearest_node_info[node.id()];

    info._nearest_node = closest_node;
    info._distance = closest_distance;
  }
}

void
NearestNodeThread::join(const NearestNodeThread & other)
{
  // Did the other one go further through the patch than this one?
  if (other._max_patch_percentage > _max_patch_percentage)
    _max_patch_percentage = other._max_patch_percentage;

  _nearest_node_info.insert(other._nearest_node_info.begin(), other._nearest_node_info.end());
}
