//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

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
