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

#ifndef SLAVENEIGHBORHOODTHREAD_H
#define SLAVENEIGHBORHOODTHREAD_H

#include "Moose.h"

// libMesh
#include "mesh_base.h"

// System
#include <set>

class SlaveNeighborhoodThread
{
public:
  SlaveNeighborhoodThread(const MeshBase & mesh,
                          const std::vector<unsigned int> & trial_master_nodes,
                          const std::vector<std::vector<unsigned int> > & node_to_elem_map);
  
  
  // Splitting Constructor
  SlaveNeighborhoodThread(SlaveNeighborhoodThread & x, Threads::split split);

  void operator() (const NodeIdRange & range);

  void join(const SlaveNeighborhoodThread & other);

  // List of the slave nodes we're actually going to keep track of
  std::vector<unsigned int> _slave_nodes;

  // The neighborhood nodes associated with each node
  std::map<unsigned int, std::vector<unsigned int> > _neighbor_nodes;

  // Elements that we need to ghost
  std::set<unsigned int> _ghosted_elems;

protected:
  // The Mesh
  const MeshBase & _mesh;

  // Nodes to search against
  const std::vector<unsigned int> & _trial_master_nodes;

  // Node to elem map
  const std::vector<std::vector<unsigned int> > & _node_to_elem_map;
};

#endif //SLAVENEIGHBORHOODTHREAD_H
