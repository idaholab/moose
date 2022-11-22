//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateTHMMeshAction.h"
#include "THMMesh.h"

registerMooseAction("ThermalHydraulicsApp", CreateTHMMeshAction, "THM:create_thm_mesh");

InputParameters
CreateTHMMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  params += THMAppInterface::validParams();

  params.addClassDescription("Creates the THM mesh and sets its base.");

  return params;
}

CreateTHMMeshAction::CreateTHMMeshAction(const InputParameters & params)
  : Action(params), THMAppInterface(params)
{
}

void
CreateTHMMeshAction::act()
{
  auto & thm_app = getTHMApp();

  // Create the THM mesh
  const std::string class_name = "THMMesh";
  InputParameters params = _factory.getValidParams(class_name);
  params.set<MooseEnum>("dim") = "3";
  params.set<unsigned int>("patch_size") = 1;
  auto thm_mesh = _factory.create<THMMesh>(class_name, "thm_mesh", params);

  // Set the THM mesh
  thm_app.setTHMMesh(thm_mesh);

  // The MeshBase must be created using the system mesh, rather than the THM
  // mesh, because we need to avoid having the allow_remote_element_removal()
  // flag out of sync between the system MooseMesh and the final MeshBase
  // created by mesh generators. Otherwise, we'll get an error in the
  // 'set_mesh_base' task in SetupMeshAction.
  auto mesh_base = _mesh->buildMeshBaseObject();

  // Since setMeshBase() would ignore whatever allow_remote_element_removal()
  // value the MeshBase has (setting it to the current value in the MooseMesh),
  // we must preserve the value by setting the MooseMesh's value to be the
  // MeshBase's value. Otherwise we can end up desyncronizing the flags in the
  // system MooseMesh and the final mesh generator MeshBase value.
  thm_mesh->allowRemoteElementRemoval(mesh_base->allow_remote_element_removal());

  thm_mesh->setMeshBase(std::move(mesh_base));
}
