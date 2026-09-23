//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MultiPointConstraint.h"

#include "MooseMesh.h"
#include "MooseVariableFieldBase.h"
#include "SystemBase.h"

#include "libmesh/node.h"

#include <algorithm>

InputParameters
MultiPointConstraint::validParams()
{
  InputParameters params = Constraint::validParams();

  // A multipoint constraint assembles neither a residual nor a Jacobian, so the tagging parameters
  // have no meaning for it. It also does not operate on a single variable: every derived class
  // declares the variables it constrains itself.
  params.suppressParameter<NonlinearVariableName>("variable");
  params.suppressParameter<MultiMooseEnum>("vector_tags");
  params.suppressParameter<MultiMooseEnum>("matrix_tags");
  params.suppressParameter<std::vector<TagName>>("extra_vector_tags");
  params.suppressParameter<std::vector<TagName>>("extra_matrix_tags");
  params.suppressParameter<std::vector<TagName>>("absolute_value_vector_tags");

  return params;
}

MultiPointConstraint::MultiPointConstraint(const InputParameters & parameters)
  : Constraint(parameters), Coupleable(this, /*nodal=*/true)
{
  if (getParam<bool>("use_displaced_mesh"))
    paramError("use_displaced_mesh",
               "The coefficients of a multipoint constraint row are built once from the "
               "undisplaced mesh, so this object does not support the displaced mesh.");
}

std::vector<dof_id_type>
MultiPointConstraint::localNodes(const BoundaryName & boundary) const
{
  std::vector<dof_id_type> local_nodes;
  for (const auto node_id : _mesh.getNodeList(_mesh.getBoundaryID(boundary)))
    if (_mesh.queryNodePtr(node_id))
      local_nodes.push_back(node_id);

  return local_nodes;
}

std::vector<MultiPointConstraint::ConstraintNode>
MultiPointConstraint::gatherBoundaryNodes(const BoundaryName & boundary,
                                          const std::vector<unsigned int> & var_numbers) const
{
  const auto sys_num = _sys.number();
  const auto n_vars = var_numbers.size();

  // Each rank contributes only the nodes of the boundary that it owns, so that every node of the
  // boundary is contributed exactly once
  std::vector<dof_id_type> ids;
  std::vector<Real> coordinates;
  std::vector<dof_id_type> dofs;
  for (const auto node_id : localNodes(boundary))
  {
    const Node & node = _mesh.nodeRef(node_id);
    if (node.processor_id() != processor_id())
      continue;

    ids.push_back(node_id);
    for (const auto d : make_range(Moose::dim))
      coordinates.push_back(node(d));
    for (const auto var_number : var_numbers)
      dofs.push_back(node.n_comp(sys_num, var_number) > 0 ? node.dof_number(sys_num, var_number, 0)
                                                          : libMesh::DofObject::invalid_id);
  }

  // The three buffers are gathered in the same rank order, so their entries stay aligned
  _communicator.allgather(ids);
  _communicator.allgather(coordinates);
  _communicator.allgather(dofs);

  std::vector<ConstraintNode> nodes(ids.size());
  for (const auto i : index_range(ids))
  {
    nodes[i].id = ids[i];
    for (const auto d : make_range(Moose::dim))
      nodes[i].point(d) = coordinates[i * Moose::dim + d];
    nodes[i].dofs.assign(dofs.begin() + i * n_vars, dofs.begin() + (i + 1) * n_vars);
  }

  // Sort by node id so that every rank builds the same rows in the same order
  std::sort(nodes.begin(),
            nodes.end(),
            [](const ConstraintNode & a, const ConstraintNode & b) { return a.id < b.id; });

  return nodes;
}

MultiPointConstraint::ConstraintNode
MultiPointConstraint::gatherSingleNode(const BoundaryName & boundary,
                                       const std::vector<unsigned int> & var_numbers) const
{
  const auto nodes = gatherBoundaryNodes(boundary, var_numbers);
  if (nodes.size() != 1)
    mooseError("The boundary '",
               boundary,
               "' holds ",
               nodes.size(),
               " nodes, but exactly one node is required.");

  return nodes.front();
}

MultiPointConstraint::ConstraintNode
MultiPointConstraint::gatherSingleNode(
    const BoundaryName & boundary,
    const std::vector<const MooseVariableFieldBase *> & variables) const
{
  std::vector<unsigned int> var_numbers;
  var_numbers.reserve(variables.size());
  for (const auto * const variable : variables)
    var_numbers.push_back(variable->number());

  const auto node = gatherSingleNode(boundary, var_numbers);

  // gatherBoundaryNodes() reports a variable that has no degree of freedom at the node with the
  // invalid id, and it reports it on every rank, so every rank raises this error
  for (const auto v : index_range(variables))
    if (node.dofs[v] == libMesh::DofObject::invalid_id)
      mooseError("The variable '",
                 variables[v]->name(),
                 "' has no degree of freedom at node ",
                 node.id,
                 " of boundary '",
                 boundary,
                 "'. Every variable this constraint ties at that node must be defined on a block "
                 "the node belongs to, such as a beam block for the rotation variables.");

  return node;
}
