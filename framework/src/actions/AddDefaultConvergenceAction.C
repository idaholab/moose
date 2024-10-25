//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddDefaultConvergenceAction.h"
#include "FEProblem.h"
#include "Executioner.h"

registerMooseAction("MooseApp", AddDefaultConvergenceAction, "add_default_convergence");

InputParameters
AddDefaultConvergenceAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Add a default Convergence object to the simulation.");
  return params;
}

AddDefaultConvergenceAction::AddDefaultConvergenceAction(const InputParameters & params)
  : Action(params)
{
}

void
AddDefaultConvergenceAction::act()
{
  if (_problem->needToAddDefaultNonlinearConvergence())
  {
    const std::string default_name = "default_nonlinear_convergence";
    _problem->setNonlinearConvergenceName(default_name);
    _problem->addDefaultNonlinearConvergence(getMooseApp().getExecutioner()->parameters());
  }
}
