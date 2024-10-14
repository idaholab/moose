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

#include "QuadSubChannelBuildMeshAction.h"
#include "SubChannelMesh.h"
#include "AddMeshGeneratorAction.h"

registerMooseAction("SubChannelApp", QuadSubChannelBuildMeshAction, "setup_mesh");
registerMooseAction("SubChannelApp", QuadSubChannelBuildMeshAction, "set_mesh_base");

InputParameters
QuadSubChannelBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Creates the infastructure necessary to buidl the subchannel mesh in "
                             "the square lattice arrangement");
  return params;
}

QuadSubChannelBuildMeshAction::QuadSubChannelBuildMeshAction(const InputParameters & params)
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
