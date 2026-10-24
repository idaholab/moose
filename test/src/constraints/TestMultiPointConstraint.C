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

#include <map>

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

std::map<dof_id_type, dof_id_type>
TestMultiPointConstraint::gatherNodeDofs() const
{
  // Each node is contributed by its owner, so remote nodes are covered
  std::vector<std::pair<dof_id_type, dof_id_type>> node_dofs;
  auto add_node = [&](const dof_id_type node_id)
  {
    const Node * const node = _mesh.queryNodePtr(node_id);
    if (!node || node->processor_id() != processor_id())
      return;

    const auto sys_num = _sys.number();
    node_dofs.emplace_back(node_id,
                           node->n_comp(sys_num, _var.number()) > 0
                               ? node->dof_number(sys_num, _var.number(), 0)
                               : libMesh::DofObject::invalid_id);
  };
  for (const auto node_id : _primary_nodes)
    add_node(node_id);
  add_node(_secondary_node);
  _communicator.allgather(node_dofs);

  std::map<dof_id_type, dof_id_type> dofs(node_dofs.begin(), node_dofs.end());
  auto check_node = [&](const dof_id_type node_id)
  {
    const auto it = dofs.find(node_id);
    if (it == dofs.end())
      mooseError("The node ", node_id, " is not in the mesh.");
    if (it->second == libMesh::DofObject::invalid_id)
      mooseError(
          "The variable '", _var.name(), "' has no degree of freedom at node ", node_id, ".");
  };
  for (const auto node_id : _primary_nodes)
    check_node(node_id);
  check_node(_secondary_node);

  return dofs;
}

void
TestMultiPointConstraint::addExactConstraints(libMesh::DofMap & dof_map) const
{
  // Collective, so every rank gathers before the ranks without the dependent node return
  const auto dofs = gatherNodeDofs();

  if (!_mesh.queryNodePtr(_secondary_node))
    return;

  libMesh::DofConstraintRow row;
  for (const auto i : index_range(_primary_nodes))
    row[libmesh_map_find(dofs, _primary_nodes[i])] += _weights[i];

  dof_map.add_constraint_row(
      libmesh_map_find(dofs, _secondary_node), row, /*forbid_constraint_overwrite=*/true);
}
