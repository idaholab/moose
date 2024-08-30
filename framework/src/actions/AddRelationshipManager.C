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
registerMooseAction("MooseApp", AddRelationshipManager, "attach_geometric_rm_final");
registerMooseAction("MooseApp", AddRelationshipManager, "attach_algebraic_rm");
registerMooseAction("MooseApp", AddRelationshipManager, "attach_coupling_rm");
registerMooseAction("MooseApp", AddRelationshipManager, "add_geometric_rm");
registerMooseAction("MooseApp", AddRelationshipManager, "add_algebraic_rm");
registerMooseAction("MooseApp", AddRelationshipManager, "add_coupling_rm");

InputParameters
AddRelationshipManager::validParams()
{
  return Action::validParams();
}

AddRelationshipManager::AddRelationshipManager(const InputParameters & params) : Action(params) {}

void
AddRelationshipManager::act()
{
  Moose::RelationshipManagerType rm_type;
  if (_current_task == "attach_geometric_rm" || _current_task == "add_geometric_rm" ||
      _current_task == "attach_geometric_rm_final")
    rm_type = Moose::RelationshipManagerType::GEOMETRIC;
  else if (_current_task == "attach_algebraic_rm" || _current_task == "add_algebraic_rm")
    rm_type = Moose::RelationshipManagerType::ALGEBRAIC;
  else
    rm_type = Moose::RelationshipManagerType::COUPLING;

  if (_current_task == "add_geometric_rm" || _current_task == "add_algebraic_rm" ||
      _current_task == "add_coupling_rm")
  {
    const auto & all_action_ptrs = _awh.allActionBlocks();
    for (const auto & action_ptr : all_action_ptrs)
      action_ptr->addRelationshipManagers(rm_type);
  }
  // Inform MooseApp that is is the final chance to attach geometric RMs
  else if (_current_task == "attach_geometric_rm_final")
    _app.attachRelationshipManagers(rm_type, true);
  else
    _app.attachRelationshipManagers(rm_type);
}
