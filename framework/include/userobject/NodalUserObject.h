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
#include "UserObject.h"
#include "BlockRestrictable.h"
#include "BoundaryRestrictable.h"
#include "Coupleable.h"
#include "MooseVariableDependencyInterface.h"
#include "TransientInterface.h"
#include "RandomInterface.h"

/**
 * A user object that runs over all the nodes and does an aggregation
 * step to compute a single value.
 */
class NodalUserObject : public UserObject,
                        public BlockRestrictable,
                        public BoundaryRestrictable,
                        public Coupleable,
                        public MooseVariableDependencyInterface,
                        public TransientInterface,
                        public RandomInterface
{
public:
  static InputParameters validParams();

  NodalUserObject(const InputParameters & parameters);

  virtual void subdomainSetup() override /*final*/;

  bool isUniqueNodeExecute() { return _unique_node_execute; }

protected:
  /// The mesh that is being iterated over
  MooseMesh & _mesh;

  /// Quadrature point index
  const unsigned int _qp;

  /// Reference to current node pointer
  const Node * const & _current_node;

  // Flag for enable/disabling multiple execute calls on nodes that share block ids
  const bool & _unique_node_execute;
};
