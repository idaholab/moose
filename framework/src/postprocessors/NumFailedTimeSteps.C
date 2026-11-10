//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NumFailedTimeSteps.h"
#include "Transient.h"

registerMooseObject("MooseApp", NumFailedTimeSteps);

InputParameters
NumFailedTimeSteps::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addClassDescription("Collects the number of failed time steps from the time stepper.");
  params.addParam<bool>("current_time_step_only",
                        false,
                        "Count only the failed solves of the current time step, which is reset "
                        "to zero when a time step converges.");
  return params;
}

NumFailedTimeSteps::NumFailedTimeSteps(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _current_time_step_only(getParam<bool>("current_time_step_only")),
    _failures_before_step(declareRestartableData<unsigned int>("failures_before_step", 0)),
    _t_step_counted(declareRestartableData<int>("t_step_counted", -1))
{
  if (_subproblem.isTransient())
  {
    _timestepper = dynamic_cast<TransientBase *>(_app.getExecutioner())->getTimeStepper();
    if (!_timestepper)
      mooseError("No user-specified time stepper in Executioner block");
  }
  else
    mooseError("Problem is not transient, and will not calculate timesteps.");
}

void
NumFailedTimeSteps::initialize()
{
  // A new time step number means the previous step converged
  if (_t_step != _t_step_counted)
  {
    _failures_before_step = _timestepper->numFailures();
    _t_step_counted = _t_step;
  }
}

Real
NumFailedTimeSteps::getValue() const
{
  if (_current_time_step_only)
    return _timestepper->numFailures() - _failures_before_step;
  return _timestepper->numFailures();
}
