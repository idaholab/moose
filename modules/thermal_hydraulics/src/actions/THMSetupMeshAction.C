//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMSetupMeshAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMSetupMeshAction, "THM:setup_mesh");
registerMooseAction("ThermalHydraulicsApp", THMSetupMeshAction, "init_mesh");

InputParameters
THMSetupMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

THMSetupMeshAction::THMSetupMeshAction(const InputParameters & params) : Action(params) {}

void
THMSetupMeshAction::act()
{
  if (_current_task == "THM:setup_mesh")
  {
    THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
    if (thm_problem)
      thm_problem->setupMesh();
  }
  else if (_current_task == "init_mesh")
  {
    // Do not do anything; it is necessary to register an action to 'init_mesh'
    // to bypass CreateMeshSetupActionsForComponents; see the return condition
    // in its act() method.
  }
}
