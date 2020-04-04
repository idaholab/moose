//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetProblem.h"
#include "LevelSetTypes.h"

#include "MultiAppTransfer.h"

registerMooseObject("LevelSetApp", LevelSetProblem);

InputParameters
LevelSetProblem::validParams()
{
  InputParameters params = FEProblem::validParams();
  params.addClassDescription("A specilized problem class that adds a custom call to "
                             "MultiAppTransfer execution to transfer adaptivity for the level set "
                             "reinitialization.");
  return params;
}

LevelSetProblem::LevelSetProblem(const InputParameters & parameters) : FEProblem(parameters) {}

void
LevelSetProblem::computeMarkers()
{
  FEProblem::computeMarkers();
  setCurrentExecuteOnFlag(LevelSet::EXEC_COMPUTE_MARKERS);
  execMultiAppTransfers(LevelSet::EXEC_COMPUTE_MARKERS, MultiAppTransfer::TO_MULTIAPP);
  setCurrentExecuteOnFlag(EXEC_NONE);
}

bool
LevelSetProblem::adaptMesh()
{
  bool adapt = FEProblem::adaptMesh();
  setCurrentExecuteOnFlag(LevelSet::EXEC_ADAPT_MESH);
  execMultiAppTransfers(LevelSet::EXEC_ADAPT_MESH, MultiAppTransfer::TO_MULTIAPP);
  setCurrentExecuteOnFlag(EXEC_NONE);
  return adapt;
}
