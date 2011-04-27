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

#include "NearestNodeThread.h"

// libmesh includes
#include "threads.h"

NearestNodeThread::NearestNodeThread(const MeshBase & mesh,
                                     std::map<unsigned int, std::vector<unsigned int> > & neighbor_nodes) :
  _mesh(mesh),
  _neighbor_nodes(neighbor_nodes)
{
}

// Splitting Constructor
NearestNodeThread::NearestNodeThread(NearestNodeThread & x, Threads::split split) :
  _mesh(x._mesh),
  _neighbor_nodes(x._neighbor_nodes)
{
}

/**
 * Save a patch of nodes that are close to each of the slave nodes to speed the search algorithm
 * TODO: This needs to be updated at some point in time.  If the hits into this data structure approach "the end"
 * then it may be time to update
 */
void
NearestNodeThread::operator() (const NodeIdRange & range)
{
  for (NodeIdRange::const_iterator nd = range.begin() ; nd != range.end(); ++nd)
  {
    unsigned int node_id = *nd;
    
    const Node & node = _mesh.node(node_id);

    const Node * closest_node = NULL;
    Real closest_distance = std::numeric_limits<Real>::max();

    const std::vector<unsigned int> & neighbor_nodes = _neighbor_nodes[node_id];

    unsigned int n_neighbor_nodes = neighbor_nodes.size();

    for(unsigned int k=0; k<n_neighbor_nodes; k++)
    {
      const Node * cur_node = &_mesh.node(neighbor_nodes[k]);
      Real distance = ((*cur_node) - node).size();

      if(distance < closest_distance)
      {
        closest_distance = distance;
        closest_node = cur_node;
      }
    }

    if(closest_distance == std::numeric_limits<Real>::max())
      mooseError("Unable to find nearest node!");

    NearestNodeLocator::NearestNodeInfo & info = _nearest_node_info[node.id()];

    info._nearest_node = closest_node;
    info._distance = closest_distance;
  }  
}

void
NearestNodeThread::join(const NearestNodeThread & other)
{
  _nearest_node_info.insert(other._nearest_node_info.begin(), other._nearest_node_info.end());
}
