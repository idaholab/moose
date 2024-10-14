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
