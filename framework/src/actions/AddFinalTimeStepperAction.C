//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddFinalTimeStepperAction.h"
#include "TimeStepper.h"
#include "Factory.h"
#include "MooseApp.h"
#include "Transient.h"

registerMooseAction("MooseApp", AddFinalTimeStepperAction, "add_final_time_stepper");

InputParameters
AddFinalTimeStepperAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addParam<std::string>(
      "type",
      "CompositionDT",
      "A string representing the Moose Object that will be built by this Action");

  params.addClassDescription("Add the Composition TimeStepper object to the simulation.");
  return params;
}

AddFinalTimeStepperAction::AddFinalTimeStepperAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddFinalTimeStepperAction::act()
{
  Transient * transient = dynamic_cast<Transient *>(_app.getExecutioner());
  _moose_object_pars.set<SubProblem *>("_subproblem") = _problem.get();
  _moose_object_pars.set<Transient *>("_executioner") = transient;

  _app.getTimeStepperSystem().setFinalTimeStepper(_moose_object_pars);
}
