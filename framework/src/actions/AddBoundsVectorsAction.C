//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddBoundsVectorsAction.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"

registerMooseAction("MooseApp", AddBoundsVectorsAction, "add_bounds_vectors");

InputParameters
AddBoundsVectorsAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Action to add vectors to nonlinear system when using the Bounds syntax.");
  return params;
}

AddBoundsVectorsAction::AddBoundsVectorsAction(const InputParameters & params) : Action(params) {}

void
AddBoundsVectorsAction::act()
{
  if (!_problem->numNonlinearSystems() && _problem->numLinearSystems())
    mooseError("Vector bounds cannot be used with only LinearSystems!");

  _problem->getNonlinearSystemBase(/*nl_sys=*/0).addVector("lower_bound", false, libMesh::GHOSTED);
  _problem->getNonlinearSystemBase(/*nl_sys=*/0).addVector("upper_bound", false, libMesh::GHOSTED);
}
