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

#ifndef NEARESTNODETHREAD_H
#define NEARESTNODETHREAD_H

#include "NearestNodeLocator.h"
#include "Moose.h"

class NearestNodeThread
{
public:
  NearestNodeThread(const MeshBase & mesh,
                    std::map<unsigned int, std::vector<unsigned int> > & neighbor_nodes);

  // Splitting Constructor
  NearestNodeThread(NearestNodeThread & x, Threads::split split);

  void operator() (const NodeIdRange & range);

  void join(const NearestNodeThread & other);

  // This is the info map we're actually filling here
  std::map<unsigned int, NearestNodeLocator::NearestNodeInfo> _nearest_node_info;

protected:
  // The Mesh
  const MeshBase & _mesh;

  // The neighborhood nodes associated with each node
  std::map<unsigned int, std::vector<unsigned int> > & _neighbor_nodes;
};

#endif //NEARESTNODETHREAD_H
