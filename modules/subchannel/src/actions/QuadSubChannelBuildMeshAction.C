#include "QuadSubChannelBuildMeshAction.h"
#include "SubChannelMesh.h"
#include "AddMeshGeneratorAction.h"

registerMooseAction("SubChannelApp", QuadSubChannelBuildMeshAction, "setup_mesh");
registerMooseAction("SubChannelApp", QuadSubChannelBuildMeshAction, "set_mesh_base");

InputParameters
QuadSubChannelBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

QuadSubChannelBuildMeshAction::QuadSubChannelBuildMeshAction(InputParameters params)
  : Action(params)
{
}

void
QuadSubChannelBuildMeshAction::act()
{
  if (_current_task == "setup_mesh")
  {
    {
      const std::string class_name = "QuadSubChannelMesh";
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
