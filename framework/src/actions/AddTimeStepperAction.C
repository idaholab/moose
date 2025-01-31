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
#include "FEProblemBase.h"
#include "TransientBase.h"

registerMooseAction("MooseApp", AddTimeStepperAction, "add_time_stepper");
registerMooseAction("MooseApp", AddTimeStepperAction, "add_time_steppers");

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
  std::string name;
  // Task: add_time_stepper corresponding to [TimeStepper] block
  if (_current_task == "add_time_stepper")
    name = _type;
  // Task: add_time_steppers corresponding to [TimeSteppers] block
  else
    name = _name;

  TransientBase * transient = dynamic_cast<TransientBase *>(_app.getExecutioner());
  if (!transient)
    mooseError("Cannot add TimeSteppers without a Transient executioner");
  _moose_object_pars.set<TransientBase *>("_executioner") = transient;

  auto ts =
      _problem->addObject<TimeStepper>(_type, name, _moose_object_pars, /* threaded = */ false)[0];

  if (name == "TimeStepper" || name == "CompositionDT")
    ts->mooseError("The user-defined time stepper name '", name, "' is a reserved name");
}
