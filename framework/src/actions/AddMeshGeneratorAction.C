//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMeshGeneratorAction.h"
#include "MooseMesh.h"
#include "MeshGenerator.h"
#include "Factory.h"
#include "MooseApp.h"

registerMooseAction("MooseApp", AddMeshGeneratorAction, "add_mesh_generator");

InputParameters
AddMeshGeneratorAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MeshGenerator object to the simulation.");
  return params;
}

AddMeshGeneratorAction::AddMeshGeneratorAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddMeshGeneratorAction::act()
{
  if (!_mesh)
    mooseError("No mesh file was supplied and no generation block was provided");

  _app.addMeshGenerator(_type, _name, _moose_object_pars);
}
