//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DumpObjectsAction.h"
#include "DumpObjectsProblem.h"

registerMooseAction("MooseApp", DumpObjectsAction, "dump_objects");

InputParameters
DumpObjectsAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

DumpObjectsAction::DumpObjectsAction(const InputParameters & params) : Action(params) {}

void
DumpObjectsAction::act()
{
  auto * const dop = dynamic_cast<DumpObjectsProblem *>(_problem.get());
  mooseAssert(dop, "Problem should be DumpObjectProblem");
  dop->printObjects();
}
