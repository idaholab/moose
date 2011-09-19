/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "SlaveNeighborhoodThread.h"

#include "AuxiliarySystem.h"
#include "Problem.h"
#include "MProblem.h"

// libmesh includes
#include "threads.h"

// System includes
#include <queue>

class ComparePair
{
public:
  bool operator()(std::pair<unsigned int, Real> & p1, std::pair<unsigned int, Real> & p2)
  {
    if(p1.second > p2.second)
      return true;

    return false;
  }
};

SlaveNeighborhoodThread::SlaveNeighborhoodThread(const MeshBase & mesh,
                                                 const std::vector<unsigned int> & trial_master_nodes,
                                                 const std::vector<std::vector<unsigned int> > & node_to_elem_map,
                                                 const unsigned int patch_size) :
  _mesh(mesh),
  _trial_master_nodes(trial_master_nodes),
  _node_to_elem_map(node_to_elem_map),
  _patch_size(patch_size)
{
}

// Splitting Constructor
SlaveNeighborhoodThread::SlaveNeighborhoodThread(SlaveNeighborhoodThread & x, Threads::split /*split*/) :
  _mesh(x._mesh),
  _trial_master_nodes(x._trial_master_nodes),
  _node_to_elem_map(x._node_to_elem_map),
  _patch_size(x._patch_size)
{
}

/**
 * Save a patch of nodes that are close to each of the slave nodes to speed the search algorithm
 * TODO: This needs to be updated at some point in time.  If the hits into this data structure approach "the end"
 * then it may be time to update
 */
void
SlaveNeighborhoodThread::operator() (const NodeIdRange & range)
{
  unsigned int processor_id = libMesh::processor_id();

  for (NodeIdRange::const_iterator nd = range.begin() ; nd != range.end(); ++nd)
  {
    unsigned int node_id = *nd;

    const Node & node = *_mesh.node_ptr(node_id);

    std::priority_queue<std::pair<unsigned int, Real>, std::vector<std::pair<unsigned int, Real> >, ComparePair> neighbors;

    unsigned int n_master_nodes = _trial_master_nodes.size();

    // Get a list, in descending order of distance, of master nodes in relation to this node
    for(unsigned int k=0; k<n_master_nodes; k++)
    {
      unsigned int master_id = _trial_master_nodes[k];
      const Node * cur_node = &_mesh.node(master_id);
      Real distance = ((*cur_node) - node).size();

      neighbors.push(std::make_pair(master_id, distance));
    }

    std::vector<unsigned int> neighbor_nodes;

    unsigned int patch_size = std::min(_patch_size, static_cast<unsigned int>(neighbors.size()));
    neighbor_nodes.resize(patch_size);

    // Grab the closest "patch_size" worth of nodes to save off
    for(unsigned int t=0; t<patch_size; t++)
    {
      std::pair<unsigned int, Real> neighbor_info = neighbors.top();
      neighbors.pop();

      neighbor_nodes[t] = neighbor_info.first;
    }

    /**
     * Now see if _this_ processor needs to keep track of this slave and it's neighbors
     * We're going to see if this processor owns the slave, any of the neighborhood nodes
     * or any of the elements connected to either set.  If it does then we're going to ghost
     * all of the elements connected to the slave node and the neighborhood nodes to this processor.
     * This is a very conservative approach that we might revisit later.
     */

    bool need_to_track = false;

    if(_mesh.node(node_id).processor_id() == processor_id)
      need_to_track = true;
    else
    {
      { // See if we own any of the elements connected to the slave node
        const std::vector<unsigned int> & elems_connected_to_node = _node_to_elem_map[node_id];

        for(unsigned int elem_id_it=0; elem_id_it < elems_connected_to_node.size(); elem_id_it++)
          if(_mesh.elem(elems_connected_to_node[elem_id_it])->processor_id() == processor_id)
          {
            need_to_track = true;
            break; // Break out of element loop
          }
      }

      if(!need_to_track)
      { // Now check the neighbor nodes to see if we own any of them
        for(unsigned int neighbor_it=0; neighbor_it < neighbor_nodes.size(); neighbor_it++)
        {
          unsigned int neighbor_node_id = neighbor_nodes[neighbor_it];

          if(_mesh.node(neighbor_node_id).processor_id() == processor_id)
            need_to_track = true;
          else // Now see if we own any of the elements connected to the neighbor nodes
          {
            const std::vector<unsigned int> & elems_connected_to_node = _node_to_elem_map[neighbor_node_id];

            for(unsigned int elem_id_it=0; elem_id_it < elems_connected_to_node.size(); elem_id_it++)
              if(_mesh.elem(elems_connected_to_node[elem_id_it])->processor_id() == processor_id)
              {
                need_to_track = true;
                break; // Break out of element loop
              }
          }

          if(need_to_track)
            break; // Breaking out of neighbor loop
        }
      }
    }

    if(need_to_track)
    {
      // Add this node as a slave node to search in the future
      _slave_nodes.push_back(node_id);

      // Set it's neighbors
      _neighbor_nodes[node_id] = neighbor_nodes;

      { // Add the elements connected to the slave node to the ghosted list
        const std::vector<unsigned int> & elems_connected_to_node = _node_to_elem_map[node_id];

        for(unsigned int elem_id_it=0; elem_id_it < elems_connected_to_node.size(); elem_id_it++)
          _ghosted_elems.insert(elems_connected_to_node[elem_id_it]);
      }

      // Now add elements connected to the neighbor nodes to the ghosted list
      for(unsigned int neighbor_it=0; neighbor_it < neighbor_nodes.size(); neighbor_it++)
      {
        const std::vector<unsigned int> & elems_connected_to_node = _node_to_elem_map[neighbor_nodes[neighbor_it]];

        for(unsigned int elem_id_it=0; elem_id_it < elems_connected_to_node.size(); elem_id_it++)
          _ghosted_elems.insert(elems_connected_to_node[elem_id_it]);
      }
    }
  }
}

void
SlaveNeighborhoodThread::join(const SlaveNeighborhoodThread & other)
{
  _slave_nodes.insert(_slave_nodes.end(), other._slave_nodes.begin(), other._slave_nodes.end());
  _neighbor_nodes.insert(other._neighbor_nodes.begin(), other._neighbor_nodes.end());
  _ghosted_elems.insert(other._ghosted_elems.begin(), other._ghosted_elems.end());
}
