//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SecondaryNeighborhoodThread.h"

#include "AuxiliarySystem.h"
#include "Problem.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include "libmesh/threads.h"

SecondaryNeighborhoodThread::SecondaryNeighborhoodThread(
    const MooseMesh & mesh,
    const std::vector<dof_id_type> & trial_primary_nodes,
    const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map,
    const unsigned int patch_size,
    KDTree & kd_tree)
  : _kd_tree(kd_tree),
    _mesh(mesh),
    _trial_primary_nodes(trial_primary_nodes),
    _node_to_elem_map(node_to_elem_map),
    _patch_size(patch_size)
{
}

// Splitting Constructor
SecondaryNeighborhoodThread::SecondaryNeighborhoodThread(SecondaryNeighborhoodThread & x,
                                                         Threads::split /*split*/)
  : _kd_tree(x._kd_tree),
    _mesh(x._mesh),
    _trial_primary_nodes(x._trial_primary_nodes),
    _node_to_elem_map(x._node_to_elem_map),
    _patch_size(x._patch_size)
{
}

/**
 * Save a patch of nodes that are close to each of the secondary nodes to speed the search algorithm
 * TODO: This needs to be updated at some point in time.  If the hits into this data structure
 * approach "the end"
 * then it may be time to update
 */
void
SecondaryNeighborhoodThread::operator()(const NodeIdRange & range)
{
  unsigned int patch_size =
      std::min(_patch_size, static_cast<unsigned int>(_trial_primary_nodes.size()));

  std::vector<std::size_t> return_index(patch_size);

  for (const auto & node_id : range)
  {
    const Node & node = _mesh.nodeRef(node_id);
    Point query_pt;
    for (const auto i : make_range(Moose::dim))
      query_pt(i) = node(i);

    /**
     * neighborSearch function takes the secondary coordinates and patch_size as
     * input and
     * finds the k (=patch_size) nearest neighbors to the secondary node from the
     * trial
     *  primary node set. The indices of the nearest neighbors are stored in the
     * array
     * return_index.
     */

    _kd_tree.neighborSearch(query_pt, patch_size, return_index);

    std::vector<dof_id_type> neighbor_nodes(return_index.size());
    for (unsigned int i = 0; i < return_index.size(); ++i)
      neighbor_nodes[i] = _trial_primary_nodes[return_index[i]];

    processor_id_type processor_id = _mesh.processor_id();

    /**
     * Now see if _this_ processor needs to keep track of this secondary and it's neighbors
     * We're going to see if this processor owns the secondary, any of the neighborhood nodes
     * or any of the elements connected to either set.  If it does then we're going to ghost
     * all of the elements connected to the secondary node and the neighborhood nodes to this
     * processor. This is a very conservative approach that we might revisit later.
     */

    bool need_to_track = false;

    if (_mesh.nodeRef(node_id).processor_id() == processor_id)
      need_to_track = true;
    else
    {
      {
        auto node_to_elem_pair = _node_to_elem_map.find(node_id);
        if (node_to_elem_pair != _node_to_elem_map.end())
        {
          const std::vector<dof_id_type> & elems_connected_to_node = node_to_elem_pair->second;

          // See if we own any of the elements connected to the secondary node
          for (const auto & dof : elems_connected_to_node)
            if (_mesh.elemPtr(dof)->processor_id() == processor_id)
            {
              need_to_track = true;
              break; // Break out of element loop
            }
        }
      }

      if (!need_to_track)
      { // Now check the neighbor nodes to see if we own any of them
        for (const auto & neighbor_node_id : neighbor_nodes)
        {
          if (_mesh.nodeRef(neighbor_node_id).processor_id() == processor_id)
            need_to_track = true;
          else // Now see if we own any of the elements connected to the neighbor nodes
          {
            auto node_to_elem_pair = _node_to_elem_map.find(neighbor_node_id);
            mooseAssert(node_to_elem_pair != _node_to_elem_map.end(),
                        "Missing entry in node to elem map");
            const std::vector<dof_id_type> & elems_connected_to_node = node_to_elem_pair->second;

            for (const auto & dof : elems_connected_to_node)
              if (_mesh.elemPtr(dof)->processor_id() == processor_id)
              {
                need_to_track = true;
                break; // Break out of element loop
              }
          }

          if (need_to_track)
            break; // Breaking out of neighbor loop
        }
      }
    }

    if (need_to_track)
    {
      // Add this node as a secondary node to search in the future
      _secondary_nodes.push_back(node_id);

      // Set it's neighbors
      _neighbor_nodes[node_id] = neighbor_nodes;

      { // Add the elements connected to the secondary node to the ghosted list
        auto node_to_elem_pair = _node_to_elem_map.find(node_id);

        if (node_to_elem_pair != _node_to_elem_map.end())
        {
          const std::vector<dof_id_type> & elems_connected_to_node = node_to_elem_pair->second;

          for (const auto & dof : elems_connected_to_node)
            _ghosted_elems.insert(dof);
        }
      }
      // Now add elements connected to the neighbor nodes to the ghosted list
      for (unsigned int neighbor_it = 0; neighbor_it < neighbor_nodes.size(); neighbor_it++)
      {
        auto node_to_elem_pair = _node_to_elem_map.find(neighbor_nodes[neighbor_it]);
        mooseAssert(node_to_elem_pair != _node_to_elem_map.end(),
                    "Missing entry in node to elem map");
        const std::vector<dof_id_type> & elems_connected_to_node = node_to_elem_pair->second;

        for (const auto & dof : elems_connected_to_node)
          _ghosted_elems.insert(dof);
      }
    }
  }
}

void
SecondaryNeighborhoodThread::join(const SecondaryNeighborhoodThread & other)
{
  _secondary_nodes.insert(
      _secondary_nodes.end(), other._secondary_nodes.begin(), other._secondary_nodes.end());
  _ghosted_elems.insert(other._ghosted_elems.begin(), other._ghosted_elems.end());
  _neighbor_nodes.insert(other._neighbor_nodes.begin(), other._neighbor_nodes.end());
}
