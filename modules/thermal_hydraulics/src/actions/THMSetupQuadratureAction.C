//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMSetupQuadratureAction.h"
#include "THMProblem.h"

registerMooseAction("ThermalHydraulicsApp", THMSetupQuadratureAction, "setup_quadrature");

InputParameters
THMSetupQuadratureAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

THMSetupQuadratureAction::THMSetupQuadratureAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
THMSetupQuadratureAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->setupQuadrature();
}
