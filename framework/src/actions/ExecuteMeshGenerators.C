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

InputParameters
ExecuteMeshGenerators::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

ExecuteMeshGenerators::ExecuteMeshGenerators(const InputParameters & params) : Action(params) {}

void
ExecuteMeshGenerators::act()
{
  // Don't do mesh generators when recovering as the master app or using master mesh! We do need
  // to run MeshGenerators for sub-apps because we don't currently have checkpoint/restart
  // information for the sub-app meshes; e.g. we just need to re-build them
  if ((_app.isRecovering() && _app.isUltimateMaster()) || _app.masterMesh() ||
      (_mesh && _mesh->isSplit()))
    return;

  _app.getMeshGeneratorSystem().executeMeshGenerators();
}
