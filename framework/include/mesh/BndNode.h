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

#ifndef BNDNODE_H
#define BNDNODE_H

#include "MooseTypes.h"
#include "libmesh/node.h"

struct BndNode
{
  BndNode(Node * node, BoundaryID bnd_id) : _node(node), _bnd_id(bnd_id) {}

  /// pointer to the node
  Node * _node;
  /// boundary id for the node
  BoundaryID _bnd_id;
};

#endif /* BNDNODE_H */
