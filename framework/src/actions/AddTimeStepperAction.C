//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddTimeStepperAction.h"
#include "TimeStepper.h"
#include "Factory.h"
#include "MooseApp.h"

registerMooseAction("MooseApp", AddTimeStepperAction, "add_time_stepper");

InputParameters
AddTimeStepperAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();

  params.addClassDescription("Add a TimeStepper object to the simulation.");
  return params;
}

AddTimeStepperAction::AddTimeStepperAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
AddTimeStepperAction::act()
{
  _app.getTimeStepperSystem().addTimeStepper(_type, _name, _moose_object_pars);
}
