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

#include "TriInterWrapperBuildMeshAction.h"
#include "InterWrapperMesh.h"
#include "AddMeshGeneratorAction.h"

registerMooseAction("SubChannelApp", TriInterWrapperBuildMeshAction, "setup_mesh");
registerMooseAction("SubChannelApp", TriInterWrapperBuildMeshAction, "set_mesh_base");

InputParameters
TriInterWrapperBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Creates the infastructure necessary to build the inter-wrapper mesh "
                             "in the triangular lattice arrangement");
  return params;
}

TriInterWrapperBuildMeshAction::TriInterWrapperBuildMeshAction(const InputParameters & params)
  : Action(params)
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
