//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddRelationshipManager.h"
#include "FEProblem.h"
#include "DisplacedProblem.h"

registerMooseAction("MooseApp", AddRelationshipManager, "attach_geometric_rm");
registerMooseAction("MooseApp", AddRelationshipManager, "attach_algebraic_rm");

template <>
InputParameters
validParams<AddRelationshipManager>()
{
  return validParams<Action>();
}

AddRelationshipManager::AddRelationshipManager(InputParameters params) : Action(params) {}

void
AddRelationshipManager::act()
{
  Moose::RelationshipManagerType rm_type =
      (_current_task == "attach_geometric_rm" ? Moose::RelationshipManagerType::GEOMETRIC
                                              : Moose::RelationshipManagerType::ALGEBRAIC);

  const auto & all_action_ptrs = _awh.allActionBlocks();
  for (const auto & action_ptr : all_action_ptrs)
    action_ptr->addRelationshipManagers(rm_type);

  _app.attachRelationshipManagers(rm_type);

  if (_current_task == "attach_algebraic_rm")
  {
    // If we're doing Algebraic then we're done adding ghosting functors
    // and we can tell the mesh that it's safe to remove remote elements again
    _mesh->getMesh().allow_remote_element_removal(true);

    if (_problem->getDisplacedProblem())
      _problem->getDisplacedProblem()->mesh().getMesh().allow_remote_element_removal(true);
  }
}
