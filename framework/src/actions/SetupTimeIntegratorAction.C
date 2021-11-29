//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupTimeIntegratorAction.h"
#include "Transient.h"
#include "Factory.h"

registerMooseAction("MooseApp", SetupTimeIntegratorAction, "setup_time_integrator");

InputParameters
SetupTimeIntegratorAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a TimeIntegrator object to the simulation.");
  return params;
}

SetupTimeIntegratorAction::SetupTimeIntegratorAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
SetupTimeIntegratorAction::act()
{
  _problem->addTimeIntegrator(_type, _name, _moose_object_pars);
}
