/********************************************************************/
/*                   DO NOT MODIFY THIS HEADER                      */
/*          Subchannel: Thermal Hydraulics Reactor Analysis         */
/*                                                                  */
/*              (c) 2022 Battelle Energy Alliance, LLC              */
/*                      ALL RIGHTS RESERVED                         */
/*                                                                  */
/*             Prepared by Battelle Energy Alliance, LLC            */
/*               Under Contract No. DE-AC07-05ID14517               */
/*               With the U. S. Department of Energy                */
/*                                                                  */
/*               See COPYRIGHT for full restrictions                */
/********************************************************************/

#include "TriSubChannelBuildMeshAction.h"
#include "SubChannelMesh.h"
#include "AddMeshGeneratorAction.h"

registerMooseAction("SubChannelApp", TriSubChannelBuildMeshAction, "setup_mesh");
registerMooseAction("SubChannelApp", TriSubChannelBuildMeshAction, "set_mesh_base");

InputParameters
TriSubChannelBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

TriSubChannelBuildMeshAction::TriSubChannelBuildMeshAction(const InputParameters & params)
  : Action(params)
{
}

void
TriSubChannelBuildMeshAction::act()
{
  if (_current_task == "setup_mesh")
  {
    {
      const std::string class_name = "TriSubChannelMesh";
      InputParameters params = _factory.getValidParams(class_name);
      _mesh = _factory.create<SubChannelMesh>(class_name, "subchannel:mesh", params);
    }
    _mesh->buildMesh();
  }
  else if (_current_task == "set_mesh_base")
  {
    auto mesh_base = _app.getMeshGeneratorMesh();
    _mesh->setMeshBase(std::move(mesh_base));
  }
}
