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

#ifndef NEARESTNODE_H
#define NEARESTNODE_H

#include "MooseSystem.h"

#include "libmesh_common.h"

#include <vector>
#include <map>

#include "mesh.h"
#include "vector_value.h"

/**
 * Finds the nearest node to each node in boundary1 to each node in boundary2 and the other way around.
 */
class NearestNode
{
public:
  NearestNode(MooseSystem & moose_system, Mesh & mesh, unsigned int boundary1, unsigned int boundary2);

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
  Node * nearestNode(unsigned int node_id);

private:
  /**
   * Data structure used to hold nearest node info.
   */
  class NearestNodeInfo
  {
  public:
    NearestNodeInfo();
    
    Node * _nearest_node;
    Real _distance;
  };

  MooseSystem & _moose_system;

  Mesh & _mesh;

  std::map<unsigned int, NearestNodeInfo> _nearest_node_info;

public:
  unsigned int _boundary1;
  unsigned int _boundary2;
};


#endif //NEARESTNODE_H
