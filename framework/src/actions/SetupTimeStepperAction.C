//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupTimeStepperAction.h"
#include "Transient.h"
#include "MooseApp.h"
#include "Factory.h"
#include "TimeStepper.h"
#include "AddTimeStepperAction.h"

registerMooseAction("MooseApp", SetupTimeStepperAction, "setup_time_steppers");

InputParameters
SetupTimeStepperAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Set up the final time stepper.");
  return params;
}

SetupTimeStepperAction::SetupTimeStepperAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
SetupTimeStepperAction::act()
{

  if (_problem->isTransient())
  {
    auto & time_stepper_system = _app.getTimeStepperSystem();

    Transient * transient = dynamic_cast<Transient *>(_app.getExecutioner());
    if (!transient)
      mooseError("You can setup time stepper only with executioners of transient type.");
    // The user add a time stepper with [TimeStepper], create one for them but post deprecation
    // notice
    if (!_awh.getActionListByName("add_time_stepper").empty())
    {
      // The user use both [TimeStepper] and [TimeSteppers] for time stepper setup, use input in
      // [TimeSteppers] block and give a warning message
      if (!_awh.getActionListByName("add_time_steppers").empty())
        mooseWarning(
            "Both [TimeStepper] and [TimeSteppers] are used to setup the time stepper. The "
            "[TimeStepper] Block will be ignored. Note [TimeStepper] will be deprecated "
            "soon. Please consider [TimeSteppers] for future use.");
      else
        mooseDeprecated(
            "The [TimeStepper] block is deprecated. Please use [TimeSteppers] instead.");
    }

    time_stepper_system.createAddedTimeSteppers();

    auto final_ts = time_stepper_system.getFinalTimeStepper();

    mooseAssert(final_ts, "Missing final TimeStepper");
    transient->setTimeStepper(final_ts);
  }
}
