#include "TriInterWrapperBuildMeshAction.h"
#include "InterWrapperMesh.h"
#include "AddMeshGeneratorAction.h"

registerMooseAction("SubChannelApp", TriInterWrapperBuildMeshAction, "setup_mesh");
registerMooseAction("SubChannelApp", TriInterWrapperBuildMeshAction, "set_mesh_base");

InputParameters
TriInterWrapperBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

TriInterWrapperBuildMeshAction::TriInterWrapperBuildMeshAction(InputParameters params) : Action(params)
{
}

void
TriInterWrapperBuildMeshAction::act()
{
  if (_current_task == "setup_mesh")
  {
    {
      const std::string class_name = "TriInterWrapperMesh";
      InputParameters params = _factory.getValidParams(class_name);
      _mesh = _factory.create<InterWrapperMesh>(class_name, "subchannel:mesh", params);
    }
    _mesh->buildMesh();
  }
  else if (_current_task == "set_mesh_base")
  {
    auto mesh_base = _app.getMeshGeneratorMesh();
    _mesh->setMeshBase(std::move(mesh_base));
  }
}
