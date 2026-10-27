//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultiPointConstraint.h"

/**
 * Ties the value of a variable at one node to a weighted sum of its values at other nodes, given
 * by id, with a single exact constraint.
 */
class TestMultiPointConstraint : public MultiPointConstraint
{
public:
  static InputParameters validParams();

  TestMultiPointConstraint(const InputParameters & parameters);

  virtual void addExactConstraints(libMesh::DofMap & dof_map) const override;

  virtual const MooseVariableBase & variable() const override;

protected:
  /**
   * @return The variable coupled to 'secondary_variable', with an error when it is a constant
   */
  const MooseVariable & coupledSecondaryVariable();

  /**
   * Gather the dof of the constrained variable at the dependent node and at every primary node,
   * from the ranks that own them. Every rank must call this.
   * @return The dof of each of those nodes, keyed by node id
   */
  std::map<dof_id_type, dof_id_type> gatherNodeDofs() const;

  /// The variable whose degrees of freedom this constraint ties together
  const MooseVariable & _var;

  /// The id of the node holding the dependent degree of freedom
  const dof_id_type _secondary_node;

  /// The ids of the nodes holding the degrees of freedom the dependent one is tied to
  const std::vector<dof_id_type> & _primary_nodes;

  /// The coefficient of each entry of _primary_nodes in the exact constraint
  const std::vector<Real> & _weights;
};
