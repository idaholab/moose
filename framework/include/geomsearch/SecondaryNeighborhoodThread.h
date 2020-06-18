//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "MooseTypes.h"
#include "NearestNodeLocator.h"
#include "KDTree.h"

// Forward declarations
class MooseMesh;
class NearestNodeLocator;
class KDTree;

class SecondaryNeighborhoodThread
{
public:
  KDTree & _kd_tree;

  SecondaryNeighborhoodThread(
      const MooseMesh & mesh,
      const std::vector<dof_id_type> & trial_primary_nodes,
      const std::map<dof_id_type, std::vector<dof_id_type>> & node_to_elem_map,
      const unsigned int patch_size,
      KDTree & _kd_tree);

  /// Splitting Constructor
  SecondaryNeighborhoodThread(SecondaryNeighborhoodThread & x, Threads::split split);

  void operator()(const NodeIdRange & range);

  void join(const SecondaryNeighborhoodThread & other);

  /// List of the secondary nodes we're actually going to keep track of
  std::vector<dof_id_type> _secondary_nodes;

  /// The neighborhood nodes associated with each node
  std::map<dof_id_type, std::vector<dof_id_type>> _neighbor_nodes;

  /// Elements that we need to ghost
  std::set<dof_id_type> _ghosted_elems;

protected:
  /// The Mesh
  const MooseMesh & _mesh;

  /// Nodes to search against
  const std::vector<dof_id_type> & _trial_primary_nodes;

  /// Node to elem map
  const std::map<dof_id_type, std::vector<dof_id_type>> & _node_to_elem_map;

  /// The number of nodes to keep
  unsigned int _patch_size;
};
