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
  if (_problem->isTransient())
  {
    Transient * transient = dynamic_cast<Transient *>(_app.getExecutioner());
    std::shared_ptr<TimeStepper> ts;
    if (!transient)
      mooseError("You can setup time stepper only with executioners of transient type.");

    // Check if the user added any time steppers
    const auto & generator_actions = _awh.getActionListByName("add_time_stepper");

    // The user added timestepper(s)
    if (!generator_actions.empty())
    {
      auto & time_stepper_system = _app.getTimeStepperSystem();
      time_stepper_system.setFinalTimeStepperName();
      time_stepper_system.createAddedTimeSteppers();
      ts = _app.getTimeStepperSystem().getFinalTimeStepper();
    }
    // The user did not add timestepper(s), so create a ConstantDT for them
    else
    {
      _moose_object_pars.set<SubProblem *>("_subproblem") = _problem.get();
      _moose_object_pars.set<Transient *>("_executioner") = transient;

      ts = _factory.create<TimeStepper>(_type, "TimeStepper", _moose_object_pars);
    }
    mooseAssert(ts, "Missing final TimeStepper");
    transient->setTimeStepper(ts);
  }
}
