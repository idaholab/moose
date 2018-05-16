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

template <>
InputParameters
validParams<AddExternalAuxVariableAction>()
{
  return validParams<Action>();
}

AddExternalAuxVariableAction::AddExternalAuxVariableAction(InputParameters params) : Action(params)
{
}

void
AddExternalAuxVariableAction::act()
{
  auto external_problem_ptr = dynamic_cast<ExternalProblem *>(_problem.get());

  if (external_problem_ptr)
    external_problem_ptr->addExternalVariables();
}
