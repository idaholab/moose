//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestMultiPointConstraint.h"

#include "MooseMesh.h"
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/dof_map.h"
#include "libmesh/node.h"

#include <algorithm>

registerMooseObject("MooseTestApp", TestMultiPointConstraint);

InputParameters
TestMultiPointConstraint::validParams()
{
  InputParameters params = MultiPointConstraint::validParams();

  params.addClassDescription(
      "Ties the value of a variable at one node to a weighted sum of its values at other nodes "
      "with a single exact constraint.");

  params.addRequiredCoupledVar("secondary_variable",
                               "The variable whose degree of freedom at 'secondary_node' is the "
                               "dependent degree of freedom of the exact constraint.");
  params.addRequiredParam<dof_id_type>(
      "secondary_node", "The id of the node holding the dependent degree of freedom.");
  params.addRequiredParam<std::vector<dof_id_type>>(
      "primary_nodes",
      "The ids of the nodes holding the degrees of freedom the dependent one is tied to.");
  params.addRequiredParam<std::vector<Real>>(
      "weights", "The coefficient of each entry of 'primary_nodes' in the exact constraint.");

  return params;
}

TestMultiPointConstraint::TestMultiPointConstraint(const InputParameters & parameters)
  : MultiPointConstraint(parameters),
    _var(coupledSecondaryVariable()),
    _secondary_node(getParam<dof_id_type>("secondary_node")),
    _primary_nodes(getParam<std::vector<dof_id_type>>("primary_nodes")),
    _weights(getParam<std::vector<Real>>("weights"))
{
  if (_weights.size() != _primary_nodes.size())
    paramError("weights",
               "The number of weights (",
               _weights.size(),
               ") must match the number of entries of 'primary_nodes' (",
               _primary_nodes.size(),
               ").");

  // Checked here, on every rank, because erroring inside the collective addExactConstraints()
  // would hang
  dof_id_type missing_node = libMesh::DofObject::invalid_id;
  for (const auto node_id : _primary_nodes)
    if (!_mesh.queryNodePtr(node_id))
      missing_node = std::min(missing_node, node_id);
  if (!_mesh.queryNodePtr(_secondary_node))
    missing_node = std::min(missing_node, _secondary_node);
  _communicator.min(missing_node);

  if (missing_node != libMesh::DofObject::invalid_id)
    mooseError("The node ",
               missing_node,
               " is not present on every process. This constraint builds its exact constraint from "
               "node ids "
               "alone, so every node it names must be present wherever the dependent node is; use "
               "a replicated mesh.");
}

const MooseVariable &
TestMultiPointConstraint::coupledSecondaryVariable()
{
  const MooseVariable * const var = getVar("secondary_variable", 0);
  if (!var)
    paramError("secondary_variable",
               "This constraint ties a degree of freedom of a variable, so 'secondary_variable' "
               "must name a variable and cannot be a constant value.");

  return *var;
}

const MooseVariableBase &
TestMultiPointConstraint::variable() const
{
  return _var;
}

dof_id_type
TestMultiPointConstraint::nodeDof(const dof_id_type node_id) const
{
  const Node * const node = _mesh.queryNodePtr(node_id);
  mooseAssert(node, "The node " << node_id << " is not present on this process");

  const auto sys_num = _sys.number();
  if (node->n_comp(sys_num, _var.number()) == 0)
    mooseError("The variable '", _var.name(), "' has no degree of freedom at node ", node_id, ".");

  return node->dof_number(sys_num, _var.number(), 0);
}

void
TestMultiPointConstraint::addExactConstraints(libMesh::DofMap & dof_map) const
{
  if (!_mesh.queryNodePtr(_secondary_node))
    return;

  libMesh::DofConstraintRow row;
  for (const auto i : index_range(_primary_nodes))
    row[nodeDof(_primary_nodes[i])] += _weights[i];

  dof_map.add_constraint_row(nodeDof(_secondary_node), row, /*forbid_constraint_overwrite=*/true);
}
