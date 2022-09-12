//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMBuildMeshAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMBuildMeshAction, "THM:build_mesh");

InputParameters
THMBuildMeshAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

THMBuildMeshAction::THMBuildMeshAction(const InputParameters & params) : Action(params) {}

void
THMBuildMeshAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->buildMesh();
}
