//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMSetUpMeshAction.h"

registerMooseAction("ThermalHydraulicsApp", THMSetUpMeshAction, "setup_mesh");
registerMooseAction("ThermalHydraulicsApp", THMSetUpMeshAction, "set_mesh_base");
registerMooseAction("ThermalHydraulicsApp", THMSetUpMeshAction, "init_mesh");

InputParameters
THMSetUpMeshAction::validParams()
{
  InputParameters params = SetupMeshAction::validParams();
  return params;
}

THMSetUpMeshAction::THMSetUpMeshAction(const InputParameters & params) : SetupMeshAction(params) {}

void
THMSetUpMeshAction::act()
{
  // This action is a SetupMeshAction, so if there is only 1, then execute;
  // else, there should be a SetupMeshAction that executes the equivalent tasks.
  if (_awh.getActions<SetupMeshAction>().size() == 1)
  {
    if (_current_task == "setup_mesh")
    {
      TIME_SECTION("THMSetUpMeshAction::act::setup_mesh", 1, "Setting Up Mesh", true);

      executeSetupMeshTask("THMMeshGeneratorMesh");
    }
    else if (_current_task == "set_mesh_base")
    {
      TIME_SECTION("THMSetUpMeshAction::act::set_mesh_base", 1, "Setting Mesh Base", true);

      executeSetMeshBaseTask();
    }
    else if (_current_task == "init_mesh")
    {
      TIME_SECTION("THMSetUpMeshAction::act::init_mesh", 1, "Initializing Mesh", true);

      executeInitMeshTask();
    }
  }
}
