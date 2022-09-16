#include "QuadInterWrapperBuildMeshAction.h"
#include "InterWrapperMesh.h"
#include "AddMeshGeneratorAction.h"

registerMooseAction("SubChannelApp", QuadInterWrapperBuildMeshAction, "setup_mesh");
registerMooseAction("SubChannelApp", QuadInterWrapperBuildMeshAction, "set_mesh_base");

InputParameters
QuadInterWrapperBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
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
