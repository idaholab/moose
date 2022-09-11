//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddExternalAuxVariableAction.h"
#include "ExternalProblem.h"

registerMooseAction("MooseApp", AddExternalAuxVariableAction, "add_external_aux_variables");

InputParameters
AddExternalAuxVariableAction::validParams()
{
  return Action::validParams();
}

AddExternalAuxVariableAction::AddExternalAuxVariableAction(const InputParameters & params)
  : Action(params)
{
}

void
AddExternalAuxVariableAction::act()
{
  auto external_problem_ptr = std::dynamic_pointer_cast<ExternalProblem>(_problem);

  if (external_problem_ptr)
    external_problem_ptr->addExternalVariables();
}
