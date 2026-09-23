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
 * Ties the value of a variable at one node to a weighted sum of its values at other nodes with a
 * single degree-of-freedom constraint row. The nodes are given by their ids, so that the row a
 * user asks for can be checked without any geometry.
 */
class TestMultiPointConstraint : public MultiPointConstraint
{
public:
  static InputParameters validParams();

  TestMultiPointConstraint(const InputParameters & parameters);

  virtual void addConstraintRows(libMesh::DofMap & dof_map) const override;

  virtual const MooseVariableBase & variable() const override;

protected:
  /**
   * @return The variable coupled to 'secondary_variable', with an error when that parameter is a
   * constant value instead of a variable
   */
  const MooseVariable & coupledSecondaryVariable();

  /**
   * @return The global degree of freedom index of the constrained variable at the node \p node_id
   */
  dof_id_type nodeDof(const dof_id_type node_id) const;

  /// The variable whose degrees of freedom this constraint ties together
  const MooseVariable & _var;

  /// The id of the node holding the dependent degree of freedom
  const dof_id_type _secondary_node;

  /// The ids of the nodes holding the degrees of freedom the dependent one is tied to
  const std::vector<dof_id_type> & _primary_nodes;

  /// The coefficient of each entry of _primary_nodes in the constraint row
  const std::vector<Real> & _weights;
};
