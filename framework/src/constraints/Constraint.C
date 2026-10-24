//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Constraint.h"

#include "FEProblemBase.h"
#include "MooseMesh.h"
#include "NodalBCBase.h"
#include "NonlinearSystemBase.h"
#include "SubProblem.h"

InputParameters
Constraint::validParams()
{
  InputParameters params = NeighborResidualObject::validParams();
  params += GeometricSearchInterface::validParams();

  params.addParam<bool>("use_displaced_mesh",
                        false,
                        "Whether or not this object should use the "
                        "displaced mesh for computation.  Note that "
                        "in the case this is true but no "
                        "displacements are provided in the Mesh block "
                        "the undisplaced mesh will still be used.");
  params.addParamNamesToGroup("use_displaced_mesh", "Advanced");

  params.registerBase("Constraint");
  params.registerSystemAttributeName("Constraint");

  return params;
}

Constraint::Constraint(const InputParameters & parameters)
  : NeighborResidualObject(parameters), GeometricSearchInterface(this)
{
}

void
Constraint::CollectiveError::raise() const
{
  std::vector<std::string> gathered_messages;
  _constraint.comm().allgather(_message, gathered_messages);

  for (const auto & message : gathered_messages)
    if (!message.empty())
      _constraint.mooseError(message);
}

NonlinearSystemBase &
Constraint::referenceSystem(const std::string & var_name) const
{
  for (const auto i : make_range(_fe_problem.numNonlinearSystems()))
    if (_fe_problem.getNonlinearSystemBase(i).hasVariable(var_name))
      return _fe_problem.getNonlinearSystemBase(i);

  mooseError("The variable '", var_name, "' is not in any nonlinear system of this problem.");
}

std::set<dof_id_type>
Constraint::nodesPinnedByNodalBCs(const std::string & var_name) const
{
  // The active lists are not filled yet when the constraints are first built
  std::set<BoundaryID> pinned_boundaries;
  for (const auto & [bnd_id, bcs] :
       referenceSystem(var_name).getNodalBCWarehouse().getBoundaryObjects())
    for (const auto & bc : bcs)
      if (bc->enabled() && bc->variable().name() == var_name)
      {
        pinned_boundaries.insert(bnd_id);
        break;
      }

  std::set<dof_id_type> pinned_nodes;
  if (!pinned_boundaries.empty())
    for (const auto & bnd_node : *_fe_problem.mesh().getBoundaryNodeRange())
      if (pinned_boundaries.count(bnd_node->_bnd_id))
        pinned_nodes.insert(bnd_node->_node->id());

  return pinned_nodes;
}
