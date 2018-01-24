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

class NearestNodeThread
{
public:
  NearestNodeThread(const MooseMesh & mesh,
                    std::map<dof_id_type, std::vector<dof_id_type>> & neighbor_nodes);

  // Splitting Constructor
  NearestNodeThread(NearestNodeThread & x, Threads::split split);

  void operator()(const NodeIdRange & range);

  void join(const NearestNodeThread & other);

  // This is the info map we're actually filling here
  std::map<dof_id_type, NearestNodeLocator::NearestNodeInfo> _nearest_node_info;

  // The furthest percentage through the patch that had to be searched (indicative of needing to
  // rebuild the patch)
  Real _max_patch_percentage;

protected:
  // The Mesh
  const MooseMesh & _mesh;

  // The neighborhood nodes associated with each node
  std::map<dof_id_type, std::vector<dof_id_type>> & _neighbor_nodes;
};

#endif // NEARESTNODETHREAD_H
