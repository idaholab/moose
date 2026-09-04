//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddMeshSmootherAction.h"

#include "FEProblem.h"

registerMooseAction("MooseApp", AddMeshSmootherAction, "add_mesh_smoother");

InputParameters
AddMeshSmootherAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MeshSmootherBase object to a simulation.");
  return params;
}

AddMeshSmootherAction::AddMeshSmootherAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddMeshSmootherAction::act()
{
  _problem->addMeshSmoother(_type, _name, _moose_object_pars);
}
