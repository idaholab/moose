//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMeshModifiersAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddMeshModifiersAction, "add_mesh_modifier");

InputParameters
AddMeshModifiersAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MeshModifier object to the simulation.");
  return params;
}

AddMeshModifiersAction::AddMeshModifiersAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddMeshModifiersAction::act()
{
  // MeshModifiers are user objects in the backend
  _problem->addUserObject(_type, _name, _moose_object_pars);
}
