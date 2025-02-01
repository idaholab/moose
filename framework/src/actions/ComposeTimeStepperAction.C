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
#include "FEProblemBase.h"
#include "CompositionDT.h"
#include "Factory.h"
#include "MooseApp.h"
#include "Transient.h"

registerMooseAction("MooseApp", ComposeTimeStepperAction, "compose_time_stepper");

InputParameters
ComposeTimeStepperAction::validParams()
{
  InputParameters params = Action::validParams();

  params += CompositionDT::compositionDTParams();

  params.addClassDescription(
      "Add the composition time stepper if multiple time steppers have been created.");

  return params;
}

ComposeTimeStepperAction::ComposeTimeStepperAction(const InputParameters & params) : Action(params)
{
}

void
ComposeTimeStepperAction::act()
{
  // Get all of the timesteppers that have been added so far
  std::vector<const TimeStepper *> timesteppers;
  _problem->theWarehouse().query().condition<AttribSystem>("TimeStepper").queryInto(timesteppers);

  for (const auto ts : timesteppers)
    if (dynamic_cast<const CompositionDT *>(ts))
      ts->mooseError(
          "You cannot construct a ", ts->type(), "; this object is for internal use only");

  // The user added multiple timesteppers in [TimeSteppers] block, so
  // create a composition timestepper to compute final time step size
  if (timesteppers.size() > 1)
  {
    const auto final_timestepper = "CompositionDT";
    auto new_params = _factory.getValidParams(final_timestepper);

    // Apply all custom parameters to CompositionDT that are
    // provided in Executioner/TimeSteppers/*
    for (const auto & param_name_value : CompositionDT::compositionDTParams())
      if (isParamValid(param_name_value.first))
        new_params.applyParameter(_pars, param_name_value.first);

    TransientBase * transient = dynamic_cast<TransientBase *>(_app.getExecutioner());
    mooseAssert(transient, "The transient executioner does not exist");
    new_params.set<TransientBase *>("_executioner") = transient;

    _problem->addObject<TimeStepper>(
        final_timestepper, final_timestepper, new_params, /* threaded = */ false);
  }
}
