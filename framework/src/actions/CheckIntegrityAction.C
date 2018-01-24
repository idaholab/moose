//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CheckIntegrityAction.h"
#include "ActionWarehouse.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<CheckIntegrityAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

CheckIntegrityAction::CheckIntegrityAction(InputParameters params) : Action(params) {}

void
CheckIntegrityAction::act()
{
  _awh.checkUnsatisfiedActions();
  if (_problem.get() != NULL)
    _problem->checkProblemIntegrity();
}
