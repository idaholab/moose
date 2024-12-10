//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "QuadInterWrapperBuildMeshAction.h"
#include "InterWrapperMesh.h"
#include "AddMeshGeneratorAction.h"

registerMooseAction("SubChannelApp", QuadInterWrapperBuildMeshAction, "setup_mesh");
registerMooseAction("SubChannelApp", QuadInterWrapperBuildMeshAction, "set_mesh_base");

InputParameters
QuadInterWrapperBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Creates the infastructure necessary to build the inter-wrapper mesh "
                             "in the square lattice arrangement");
  return params;
}

QuadInterWrapperBuildMeshAction::QuadInterWrapperBuildMeshAction(const InputParameters & params)
  : Action(params)
{
}

void
QuadInterWrapperBuildMeshAction::act()
{
  if (_current_task == "setup_mesh")
  {
    {
      const std::string class_name = "QuadInterWrapperMesh";
      InputParameters params = _factory.getValidParams(class_name);
      _mesh = _factory.create<InterWrapperMesh>(class_name, "interwrapper:mesh", params);
    }
    _mesh->buildMesh();
  }
  else if (_current_task == "set_mesh_base")
  {
    auto mesh_base = _app.getMeshGeneratorMesh();
    _mesh->setMeshBase(std::move(mesh_base));
  }
}
