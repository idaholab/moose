//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// Moose
#include "Restartable.h"
#include "PerfGraphInterface.h"

// Forward declarations
class SubProblem;
class MooseMesh;

/**
 * Finds the nearest node to each node in boundary1 to each node in boundary2 and the other way
 * around.
 */
class NearestNodeLocator : public Restartable, public PerfGraphInterface
{
public:
  NearestNodeLocator(SubProblem & subproblem,
                     MooseMesh & mesh,
                     BoundaryID boundary1,
                     BoundaryID boundary2);

  ~NearestNodeLocator();

  /**
   * This is the main method that is going to start the search.
   */
  void findNodes();

  /**
   * Completely redo the search from scratch.
   * Most likely called because of mesh adaptivity.
   */
  void reinit();

  /**
   * Valid to call this after findNodes() has been called to get the distance to the nearest node.
   */
  Real distance(dof_id_type node_id);

  /**
   * Valid to call this after findNodes() has been called to get a pointer to the nearest node.
   */
  const Node * nearestNode(dof_id_type node_id);

  /**
   * Returns the list of secondary nodes this Locator is tracking.
   */
  std::vector<dof_id_type> & secondaryNodes() { return _secondary_nodes; }

  /**
   * Returns the NodeIdRange of secondary nodes to be used for calling threaded
   * functions operating on the secondary nodes.
   */
  NodeIdRange & secondaryNodeRange() { return *_secondary_node_range; }

  /**
   * Reconstructs the KDtree, updates the patch for the nodes in secondary_nodes,
   * and updates the closest neighbor for these nodes in nearest node info.
   */
  void updatePatch(std::vector<dof_id_type> & secondary_nodes);

  /**
   * Updates the ghosted elements at the start of the time step for iterion
   * patch update strategy.
   */
  void updateGhostedElems();

  /**
   * Data structure used to hold nearest node info.
   */
  class NearestNodeInfo
  {
  public:
    NearestNodeInfo();

    const Node * _nearest_node;
    Real _distance;
  };

protected:
  SubProblem & _subproblem;

  MooseMesh & _mesh;

  std::unique_ptr<NodeIdRange> _secondary_node_range;

public:
  std::map<dof_id_type, NearestNodeInfo> _nearest_node_info;

  BoundaryID _boundary1;
  BoundaryID _boundary2;

  bool _first;
  std::vector<dof_id_type> _secondary_nodes;

  std::map<dof_id_type, std::vector<dof_id_type>> _neighbor_nodes;

  // The following parameter controls the patch size that is searched for each nearest neighbor
  static const unsigned int _patch_size;

  // Contact patch update strategy
  const Moose::PatchUpdateType _patch_update_strategy;

  // The furthest through the patch that had to be searched for any node last time
  Real _max_patch_percentage;

  // The list of ghosted elements added during a time step for iteration patch update strategy
  std::vector<dof_id_type> _new_ghosted_elems;
};
