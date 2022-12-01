//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMBuildMeshAction.h"
#include "Component.h"

registerMooseAction("ThermalHydraulicsApp", THMBuildMeshAction, "THM:build_mesh");

InputParameters
THMBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  params += THMAppInterface::validParams();

  params.addClassDescription("Builds the THM mesh.");

  return params;
}

THMBuildMeshAction::THMBuildMeshAction(const InputParameters & params)
  : Action(params), THMAppInterface(params)
{
}

void
THMBuildMeshAction::act()
{
  auto & thm_app = getTHMApp();
  auto & components = thm_app.getComponents();

  if (components.size() == 0)
    return;

  // perform any pre-mesh-setup initialization
  for (auto && component : components)
    component->executePreSetupMesh();

  // build mesh
  for (auto && component : components)
    component->executeSetupMesh();

  // Make sure all node sets have their corresponding side sets
  auto & thm_mesh = thm_app.getTHMMesh();
  if (thm_mesh->getMesh().get_boundary_info().n_nodeset_conds() > 0)
    thm_mesh->getMesh().get_boundary_info().build_side_list_from_node_list();
}
