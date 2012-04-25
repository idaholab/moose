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
#include "Moose.h"
#include "MooseMesh.h"
#include "vector_value.h"

// libMesh
#include "libmesh_common.h"

// System
#include <vector>
#include <map>


class MooseMesh;
class SubProblem;

/**
 * Finds the nearest node to each node in boundary1 to each node in boundary2 and the other way around.
 */
class NearestNodeLocator
{
public:
  NearestNodeLocator(SubProblem & subproblem, MooseMesh & mesh, BoundaryID boundary1, BoundaryID boundary2);

  ~NearestNodeLocator();

  /**
   * This is the main method that is going to start the search.
   */
  void findNodes();

  /**
   * Valid to call this after findNodes() has been called to get the distance to the nearest node.
   */
  Real distance(unsigned int node_id);

  /**
   * Valid to call this after findNodes() has been called to get a pointer to the nearest node.
   */
  const Node * nearestNode(unsigned int node_id);

  /**
   * Returns the list of slave nodes this Locator is tracking.
   */
  std::vector<unsigned int> & slaveNodes() { return _slave_nodes; }

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
  std::map<unsigned int, NearestNodeInfo> _nearest_node_info;

  BoundaryID _boundary1;
  BoundaryID _boundary2;

  bool _first;
  std::vector<unsigned int> _slave_nodes;

  std::map<unsigned int, std::vector<unsigned int> > _neighbor_nodes;

  // The following parameter controls the patch size that is searched for each nearest neighbor
  static const unsigned int _patch_size;
};

#endif //NEARESTNODELOCATOR_H
