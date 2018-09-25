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

registerMooseAction("MooseApp", AddRelationshipManager, "add_algebraic_rm");

registerMooseAction("MooseApp", AddRelationshipManager, "add_geometric_rm");

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
      (_current_task == "add_geometric_rm" ? Moose::RelationshipManagerType::GEOMETRIC
                                           : Moose::RelationshipManagerType::ALGEBRAIC);

  const auto & all_action_ptrs = _awh.allActionBlocks();
  for (const auto & action_ptr : all_action_ptrs)
    action_ptr->addRelationshipManagers(rm_type);

  _app.attachRelationshipManagers(rm_type);
}
