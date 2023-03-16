//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "StepUserObject.h"
#include <limits>
#include <algorithm>

registerMooseObject("TensorMechanicsApp", StepUserObject);

InputParameters
StepUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<std::vector<Real>>(
      "step_start_times",
      "The beginning of step times. The number of steps is inferred from the number of times. One "
      "step is defined by its start time; and its end time is taken from the start time of the "
      "next step (if it exists). This list needs to be in ascending value order.");

  params.addParam<Real>("total_time_interval",
                        "The total time interval in which the steps take place. This option needs "
                        "to be used together with the 'number_steps'.");
  params.addParam<unsigned int>(
      "number_steps",
      "Total number of steps in the total time inteval (provided as total_time_interval).");

  params.addParam<std::vector<Real>>(
      "step_durations",
      "The durations of the steps. 'n' of step time intervals define 'n+1' steps "
      "starting at time equals zero.");

  return params;
}

StepUserObject::StepUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _times(0),
    _step_durations(0),
    _total_time_interval(0),
    _number_steps(0)
{
  const bool is_step_times = isParamSetByUser("step_start_times");
  const bool is_interval_and_steps =
      isParamSetByUser("total_time_interval") && isParamSetByUser("number_steps");
  const bool is_step_durations = isParamSetByUser("step_durations");

  if (is_step_times)
  {
    _times = getParam<std::vector<Real>>("step_start_times");
    if (!std::is_sorted(_times.begin(), _times.end()))
      paramError(
          "step_start_times",
          "start times for StepUserObject are not provided in ascending order. Please revise "
          "your input.");

    mooseInfo("Step start times are used to define simulation steps in ", name(), ".");
  }
  else if (is_interval_and_steps)
  {
    _total_time_interval = getParam<Real>("total_time_interval");
    _number_steps = getParam<unsigned int>("number_steps");
    _times.resize(_number_steps + 1);

    for (const auto i : index_range(_times))
    {
      if (i == 0)
        _times[0] = 0.0;
      else
        _times[i] = _total_time_interval / _number_steps + _times[i - 1];
    }

    mooseInfo("The total time interval and the provided number of steps are used to define "
              "simulation steps in ",
              name(),
              ".");
  }
  else if (is_step_durations)
  {
    _step_durations = getParam<std::vector<Real>>("step_durations");
    _times.resize(_step_durations.size() + 1);
    for (const auto i : index_range(_times))
    {
      if (i == 0)
        _times[i] = 0.0;
      else
        _times[i] = _step_durations[i - 1] + _times[i - 1];
    }
    mooseInfo("Step durations are used to define simulation steps in ", name(), ".");
  }
  else
    paramError("step_start_times",
               "Plese provide 'step_start_times' or 'step_durations' or 'total_time_interval' and "
               "'number_steps' to define simulation loading steps.");
}

Real
StepUserObject::getStartTime(const unsigned int & step) const
{
  if (_times.size() <= step)
    mooseError("StepUserObject was called with a wrong step number");

  return _times[step];
}

Real
StepUserObject::getEndTime(const unsigned int & step) const
{
  Real end_time(0);

  if (_times.size() > step + 1)
    end_time = _times[step + 1];
  else if (_times.size() == step + 1)
    end_time = std::numeric_limits<double>::max();
  else
    mooseError("StepUserObject was called with a wrong step number");

  return end_time;
}

unsigned int
StepUserObject::getStep(const Real & time) const
{
  int which_step = 0;

  for (const auto i : index_range(_times))
  {
    if (i + 1 == _times.size())
      return i;

    which_step = i;
    if (time >= _times[i] && time < _times[i + 1])
      return which_step;
  }

  return which_step;
}

void
StepUserObject::initialize()
{
}
void
StepUserObject::execute()
{
}
void
StepUserObject::finalize()
{
}
