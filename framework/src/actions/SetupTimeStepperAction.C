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

registerMooseAction("MooseApp", SetupTimeStepperAction, "setup_time_stepper");

InputParameters
SetupTimeStepperAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();

  params.addParam<std::string>(
      "type",
      "ConstantDT",
      "A string representing the Moose Object that will be built by this Action");

  params.addParam<Real>("dt", 1, "Size of the time step");

  params.addClassDescription("Add and initialize a TimeStepper object to the simulation.");
  return params;
}

SetupTimeStepperAction::SetupTimeStepperAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
SetupTimeStepperAction::act()
{
  const auto apply_ts_params = [this](auto & params)
  {
    params.template set<SubProblem *>("_subproblem") = _problem.get();
    params.template set<Transient *>("_executioner") =
        dynamic_cast<Transient *>(_app.getExecutioner());
  };

  if (_problem->isTransient())
  {
    auto & time_stepper_system = _app.getTimeStepperSystem();

    Transient * transient = dynamic_cast<Transient *>(_app.getExecutioner());
    if (!transient)
      mooseError("You can setup time stepper only with executioners of transient type.");

    std::string final_timestepper;
    const auto user_set_final_timestepper = transient->isParamValid("final_time_stepper");

    // The user did not add any time steppers, so create one for them
    if (!time_stepper_system.getNumAddedTimeSteppers())
    {
      final_timestepper = _type;
      apply_ts_params(_moose_object_pars);

      time_stepper_system.addTimeStepper(_type, final_timestepper, _moose_object_pars);
      if (user_set_final_timestepper)
        transient->paramError("final_time_stepper",
                              "Cannot use this parameter because no timestepper(s) were defined");
    }
    // The user added multiple timesteppers and did not set a final one, so
    // create a composition timestepper for them
    else if (time_stepper_system.getNumAddedTimeSteppers() > 1 && !user_set_final_timestepper)
    {
      final_timestepper = "CompositionDT";
      auto params = _factory.getValidParams("CompositionDT");
      apply_ts_params(params);
      time_stepper_system.addTimeStepper("CompositionDT", final_timestepper, params);
    }

    if (user_set_final_timestepper)
      final_timestepper = transient->getParam<std::string>("final_time_stepper");

    time_stepper_system.createAddedTimeSteppers(final_timestepper);

    auto final_ts = time_stepper_system.getFinalTimeStepper();
    mooseAssert(final_ts, "Missing final TimeStepper");
    transient->setTimeStepper(final_ts);
  }
}
