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

#ifndef NEARESTNODELOCATOR_H
#define NEARESTNODELOCATOR_H

// Moose
#include "Restartable.h"

// Forward declarations
class SubProblem;
class MooseMesh;

/**
 * Finds the nearest node to each node in boundary1 to each node in boundary2 and the other way
 * around.
 */
class NearestNodeLocator : public Restartable
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
   * Returns the list of slave nodes this Locator is tracking.
   */
  std::vector<dof_id_type> & slaveNodes() { return _slave_nodes; }

  /**
   * Returns the NodeIdRange of slave nodes to be used for calling threaded
   * functions operating on the slave nodes.
   */
  NodeIdRange & slaveNodeRange() { return *_slave_node_range; }

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

  NodeIdRange * _slave_node_range;

public:
  std::map<dof_id_type, NearestNodeInfo> _nearest_node_info;

  BoundaryID _boundary1;
  BoundaryID _boundary2;

  bool _first;
  std::vector<dof_id_type> _slave_nodes;

  std::map<dof_id_type, std::vector<dof_id_type>> _neighbor_nodes;

  // The following parameter controls the patch size that is searched for each nearest neighbor
  static const unsigned int _patch_size;

  // The furthest through the patch that had to be searched for any node last time
  Real _max_patch_percentage;
};

#endif // NEARESTNODELOCATOR_H
