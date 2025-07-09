//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnalysisStepUserObject.h"
#include "MooseUtils.h"
#include <limits>
#include <algorithm>

registerMooseObject("SolidMechanicsApp", AnalysisStepUserObject);

InputParameters
AnalysisStepUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addClassDescription(
      "Maps the time steps and the load step simulation times. It can be "
      "used in either direction depending on the simulation setup. It "
      "generates steps to be used in StepPeriod to control the enabled/disabled state of objects "
      "with user-provided simulation steps.");
  params.addParam<std::vector<Real>>(
      "step_start_times",
      "The beginning of step times. The number of steps is inferred from the number of times. One "
      "step is defined by its start time; and its end time is taken from the start time of the "
      "next step (if it exists). This list needs to be in ascending value order.");
  params.addParam<std::vector<Real>>(
      "step_end_times",
      "The end of step times. The number of steps is inferred from the number of times. One "
      "step is defined by the interval between previous start time and the next. The first step "
      "is assumed to start at time zero. This list needs to be in ascending value order.");
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

  params.addParam<bool>(
      "set_sync_times", false, "Whether to make the output times include the step times.");

  return params;
}

AnalysisStepUserObject::AnalysisStepUserObject(const InputParameters & parameters)
  : GeneralUserObject(parameters),
    _times(0),
    _step_durations(0),
    _total_time_interval(0),
    _number_steps(0)
{
  const bool is_step_start_times = isParamSetByUser("step_start_times");
  const bool is_step_end_times = isParamSetByUser("step_end_times");
  const bool is_interval_and_steps =
      isParamSetByUser("total_time_interval") && isParamSetByUser("number_steps");
  const bool is_step_durations = isParamSetByUser("step_durations");

  // check for valid user input
  if (int(is_step_start_times) + int(is_step_end_times) + int(is_interval_and_steps) +
          int(is_step_durations) >
      1)
    mooseError("In AnalysisStepUserObject, only one of 'step_start_times', 'step_end_times', "
               "'total_time_interval', and 'step_durations' can be set");
  if ((isParamSetByUser("total_time_interval") && !isParamSetByUser("number_steps")) ||
      (!isParamSetByUser("total_time_interval") && isParamSetByUser("number_steps")))
    mooseError(
        "In AnalysisStepUserObject, both 'total_time_interval' and 'number_steps' need both be set.");

  // define step times
  if (is_step_start_times)
  {
    _times = getParam<std::vector<Real>>("step_start_times");
    if (!std::is_sorted(_times.begin(), _times.end()))
      paramError(
          "step_start_times",
          "start times for AnalysisStepUserObject are not provided in ascending order. Please revise "
          "your input.");

    mooseInfo("Step start times are used to define simulation steps in ", name(), ".");
  }
  else if (is_step_end_times)
  {
    _times = getParam<std::vector<Real>>("step_end_times");
    if (!std::is_sorted(_times.begin(), _times.end()))
      paramError("step_end_times",
                 "end times for AnalysisStepUserObject are not provided in ascending order. Please revise "
                 "your input.");
    _times.insert(_times.begin(), 0.0);

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

  // set sync times
  if (getParam<bool>("set_sync_times"))
  {
    std::set<Real> & sync_times = _app.getOutputWarehouse().getSyncTimes();
    for (const auto t : _times)
      sync_times.insert(t);
  }
}

Real
AnalysisStepUserObject::getStartTime(const unsigned int & step) const
{
  if (_times.size() <= step)
    mooseError("AnalysisStepUserObject was called with a wrong step number");

  return _times[step];
}

Real
AnalysisStepUserObject::getEndTime(const unsigned int & step) const
{
  Real end_time(0);

  if (_times.size() > step + 1)
    end_time = _times[step + 1];
  else if (_times.size() == step + 1)
    end_time = std::numeric_limits<double>::max();
  else
    mooseError("AnalysisStepUserObject was called with a wrong step number");

  return end_time;
}

unsigned int
AnalysisStepUserObject::getStep(const Real & time) const
{
  int which_step = 0;

  for (const auto i : index_range(_times))
  {
    if (i + 1 == _times.size())
      return i;

    which_step = i;
    if (MooseUtils::absoluteFuzzyGreaterEqual(time, _times[i]) &&
        MooseUtils::absoluteFuzzyLessThan(time, _times[i + 1]))
      return which_step;
  }

  return which_step;
}

void
AnalysisStepUserObject::initialize()
{
}
void
AnalysisStepUserObject::execute()
{
}
void
AnalysisStepUserObject::finalize()
{
}
