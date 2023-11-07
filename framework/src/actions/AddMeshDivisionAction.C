//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMeshDivisionAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddMeshDivisionAction, "add_mesh_division");

InputParameters
AddMeshDivisionAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MeshDivision object to the simulation.");
  return params;
}

AddMeshDivisionAction::AddMeshDivisionAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddMeshDivisionAction::act()
{
  _problem->addMeshDivision(_type, _name, _moose_object_pars);
}
