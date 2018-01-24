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

template <>
InputParameters
validParams<AddBoundsVectorsAction>()
{
  return validParams<Action>();
}

AddBoundsVectorsAction::AddBoundsVectorsAction(InputParameters params) : Action(params) {}

void
AddBoundsVectorsAction::act()
{
  _problem->getNonlinearSystemBase().addVector("lower_bound", false, GHOSTED);
  _problem->getNonlinearSystemBase().addVector("upper_bound", false, GHOSTED);
}
