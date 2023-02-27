//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CreateAddedMeshGenerators.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", CreateAddedMeshGenerators, "create_added_mesh_generators");

InputParameters
CreateAddedMeshGenerators::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

CreateAddedMeshGenerators::CreateAddedMeshGenerators(const InputParameters & params)
  : Action(params)
{
}

void
CreateAddedMeshGenerators::act()
{
  _app.getMeshGeneratorSystem().createAddedMeshGenerators();
}
