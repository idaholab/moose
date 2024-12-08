//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"

namespace libMesh
{
class Node;
}

struct BndNode
{
  BndNode(libMesh::Node * node, BoundaryID bnd_id) : _node(node), _bnd_id(bnd_id) {}

  /// pointer to the node
  libMesh::Node * _node;
  /// boundary id for the node
  BoundaryID _bnd_id;
};
