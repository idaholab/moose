//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateMeshDefaultAction.h"

#include "Factory.h"
#include "ActionWarehouse.h"
#include "SetupMeshAction.h"

registerMooseAction("MooseApp", CreateMeshDefaultAction, "create_mesh_default");

template <>
InputParameters
validParams<CreateMeshDefaultAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

CreateMeshDefaultAction::CreateMeshDefaultAction(InputParameters parameters) : Action(parameters) {}

void
CreateMeshDefaultAction::act()
{
  auto p = _awh.getActionByTask<SetupMeshAction>("setup_mesh");

  // If there is no SetupMeshAction, then there is no Mesh block. Let's go ahead and create a
  // SetupMeshAction with an underlying default Mesh type of MeshGeneratorMesh. This will allow
  // an input file to contain only MeshGenerators that will be responsible for building the Mesh.
  if (!p)
  {
    const std::string default_mesh_type_no_block("MeshGeneratorMesh");

    auto params = _action_factory.getValidParams("SetupMeshAction");
    params.set<std::string>("type") = default_mesh_type_no_block;

    auto action_obj = _action_factory.create("SetupMeshAction", "Mesh", params);
    _awh.addActionBlock(action_obj);
  }
}
