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

#include "libmesh/dof_map.h"
#include "libmesh/node.h"

#include <algorithm>

registerMooseObject("MooseTestApp", TestMultiPointConstraint);

InputParameters
TestMultiPointConstraint::validParams()
{
  InputParameters params = MultiPointConstraint::validParams();

  params.addClassDescription(
      "Ties the value of a variable at each node of a boundary to its value at the coincident "
      "node of another boundary with exact constraints.");

  params.addRequiredCoupledVar("secondary_variable",
                               "The variable whose degrees of freedom this constraint ties.");
  params.addRequiredParam<BoundaryName>(
      "secondary_boundary", "The boundary whose nodes hold the dependent degrees of freedom.");
  params.addRequiredParam<BoundaryName>(
      "primary_boundary",
      "The boundary whose nodes hold the degrees of freedom that the dependent ones are tied to. "
      "Each node of 'secondary_boundary' must coincide with one of its nodes.");
  params.set<std::string>(system_variable_param) = "secondary_variable";

  return params;
}

TestMultiPointConstraint::TestMultiPointConstraint(const InputParameters & parameters)
  : MultiPointConstraint(parameters),
    _variables(getConstrainedVars("secondary_variable")),
    _secondary_boundary(getParam<BoundaryName>("secondary_boundary")),
    _primary_boundary(getParam<BoundaryName>("primary_boundary"))
{
}

void
TestMultiPointConstraint::addExactConstraints(libMesh::DofMap & dof_map) const
{
  // Both gathers are collective, so every rank does them before it skips the nodes of other ranks
  const auto primary_nodes = gatherBoundaryNodes(_primary_boundary, _variables);
  const auto secondary_nodes = gatherBoundaryNodes(_secondary_boundary, _variables);

  for (const auto & secondary : secondary_nodes)
  {
    // The owner adds the exact constraint, and libMesh sends it to the ranks that need it
    const Node * const node = _mesh.queryNodePtr(secondary.id);
    if (!node || node->processor_id() != processor_id())
      continue;

    // libMesh::TOLERANCE is the geometric tolerance that libMesh uses to compare points
    const auto primary = std::find_if(
        primary_nodes.begin(),
        primary_nodes.end(),
        [&secondary](const ConstraintNode & candidate)
        { return candidate.point.absolute_fuzzy_equals(secondary.point, libMesh::TOLERANCE); });
    if (primary == primary_nodes.end())
      mooseError("No node of boundary '",
                 _primary_boundary,
                 "' coincides with node ",
                 secondary.id,
                 " of boundary '",
                 _secondary_boundary,
                 "'.");

    if (secondary.dofs[0] == libMesh::DofObject::invalid_id ||
        primary->dofs[0] == libMesh::DofObject::invalid_id)
      mooseError("The variable '",
                 _variables[0]->name(),
                 "' has no degree of freedom at node ",
                 secondary.id,
                 " or at node ",
                 primary->id,
                 ".");

    libMesh::DofConstraintRow row;
    row[primary->dofs[0]] = 1.0;
    dof_map.add_constraint_row(secondary.dofs[0], row, /*forbid_constraint_overwrite=*/true);
  }
}
