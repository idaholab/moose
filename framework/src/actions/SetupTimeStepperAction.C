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
#include "FEProblemBase.h"
#include "CompositionDT.h"

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
  if (Transient * transient = dynamic_cast<Transient *>(_app.getExecutioner()))
  {
    std::vector<TimeStepper *> timesteppers;
    _problem->theWarehouse().query().condition<AttribSystem>("TimeStepper").queryInto(timesteppers);

    // No timestepper(s) were added by the user, so add a default one
    if (timesteppers.empty())
    {
      const auto ts_name = "ConstantDT";
      auto params = _factory.getValidParams(ts_name);
      params.set<Transient *>("_executioner") = transient;

      if (!transient->parameters().isParamSetByAddParam("end_time") &&
          !transient->parameters().isParamSetByAddParam("num_steps") &&
          transient->parameters().isParamSetByAddParam("dt"))
        params.set<Real>("dt") =
            (transient->getParam<Real>("end_time") - transient->getParam<Real>("start_time")) /
            static_cast<Real>(transient->getParam<unsigned int>("num_steps"));
      else
        params.set<Real>("dt") = transient->getParam<Real>("dt");

      params.set<bool>("reset_dt") = transient->getParam<bool>("reset_dt");

      auto ts =
          _problem->addObject<TimeStepper>(ts_name, ts_name, params, /* threaded = */ false)[0];
      transient->setTimeStepper(*ts);
    }
    // TimeStepper(s) were added by the user
    else
    {
      // The user add a time stepper with [TimeStepper] or [TimeSteppers], create one for them
      auto no_time_stepper = _awh.getActionListByName("add_time_stepper").empty();
      auto no_time_steppers = _awh.getActionListByName("add_time_steppers").empty();
      if (!no_time_stepper || !no_time_steppers)
      {
        std::vector<TimeStepper *> timesteppers;
        _problem->theWarehouse()
            .query()
            .condition<AttribSystem>("TimeStepper")
            .queryInto(timesteppers);

        mooseAssert(timesteppers.size(), "Timesteppers not found");

        if (timesteppers.size() == 1)
          transient->setTimeStepper(*timesteppers[0]);
        else
          for (auto ts : timesteppers)
            if (dynamic_cast<CompositionDT *>(ts))
              transient->setTimeStepper(*ts);

        mooseAssert(transient->getTimeStepper(), "Not set");
      }
      // The user used both [TimeStepper] and [TimeSteppers] for time stepper setup, use input in
      // [TimeSteppers] block and give an error message
      if (!no_time_stepper && !no_time_steppers)
        mooseError("Both [TimeStepper] and [TimeSteppers] are used to setup the time stepper. The "
                   "[TimeStepper] block will be ignored. Note [TimeStepper] will be deprecated "
                   "soon. Please consider [TimeSteppers] for future use.");
    }
  }
}
