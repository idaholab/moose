//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExecuteMeshGenerators.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", ExecuteMeshGenerators, "execute_mesh_generators");

template <>
InputParameters
validParams<ExecuteMeshGenerators>()
{
  InputParameters params = validParams<Action>();
  return params;
}

ExecuteMeshGenerators::ExecuteMeshGenerators(InputParameters params) : Action(params) {}

void
ExecuteMeshGenerators::act()
{
  _app.executeMeshGenerators();
}
