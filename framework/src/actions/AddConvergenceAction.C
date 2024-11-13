//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddConvergenceAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddConvergenceAction, "add_convergence");

InputParameters
AddConvergenceAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Convergence object to the simulation.");
  return params;
}

AddConvergenceAction::AddConvergenceAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddConvergenceAction::act()
{
  _problem->addConvergence(_type, _name, _moose_object_pars);
}
