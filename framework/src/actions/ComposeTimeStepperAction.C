//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComposeTimeStepperAction.h"
#include "TimeStepper.h"
#include "Factory.h"
#include "MooseApp.h"

registerMooseAction("MooseApp", ComposeTimeStepperAction, "compose_time_stepper");

InputParameters
ComposeTimeStepperAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();

  params.addParam<std::string>(
      "type",
      "ConstantDT",
      "A string representing the Moose Object that will be built by this Action");

  params.addParam<Real>("dt", 1, "Size of the time step");

  params.addParam<std::vector<std::string>>(
      "lower_bound",
      "The input TimeSteppers to compose the lower bound time "
      "step size.  This can either be N timesteppers or one "
      "timestepper.");

  params.addClassDescription(
      "Add the composition time stepper if multiple time steppers have been created.");
  return params;
}

ComposeTimeStepperAction::ComposeTimeStepperAction(const InputParameters & params)
  : MooseObjectAction(params)
{
}

void
ComposeTimeStepperAction::act()
{
  auto & time_stepper_system = _app.getTimeStepperSystem();
  // The user added multiple timesteppers in [TimeSteppers] block, so
  // create a composition timestepper to compute final time step size
  if (!_awh.getActionListByName("add_time_steppers").empty())
  {
    if (time_stepper_system.getNumAddedTimeSteppers() > 1)
    {
      auto final_timestepper = "CompositionDT";
      auto new_params = _factory.getValidParams("CompositionDT");
      if (isParamValid("lower_bound"))
        new_params.set<std::vector<std::string>>("lower_bound") =
            getParam<std::vector<std::string>>("lower_bound");
      time_stepper_system.addTimeStepper("CompositionDT", final_timestepper, new_params);
    }
  }
}
