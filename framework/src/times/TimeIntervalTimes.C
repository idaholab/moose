//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeIntervalTimes.h"
#include "MooseUtils.h"
#include "Transient.h"

registerMooseObject("MooseApp", TimeIntervalTimes);

InputParameters
TimeIntervalTimes::validParams()
{
  InputParameters params = Times::validParams();
  params.addClassDescription("Times between a start time and end time with a fixed time interval.");
  params.addRequiredRangeCheckedParam<Real>(
      "time_interval", "time_interval > 0", "Time interval between times");
  params.addParam<Real>("start_time",
                        "Start time. If not provided, the simulation start time is used.");
  params.addParam<Real>("end_time", "End time. If not provided, the simulation end time is used.");
  params.addParam<bool>(
      "always_include_end_time",
      true,
      "If true, includes the end time even if the last time interval would be partial");

  // Times are known for all processes already
  params.set<bool>("auto_broadcast") = false;

  return params;
}

TimeIntervalTimes::TimeIntervalTimes(const InputParameters & parameters) : Times(parameters)
{
  // Get start time
  Real start_time;
  if (isParamValid("start_time"))
    start_time = getParam<Real>("start_time");
  else
  {
    if (auto transient = dynamic_cast<TransientBase *>(_app.getExecutioner()))
      start_time = transient->getStartTime();
    else
      mooseError("If the parameter 'start_time' is not provided, the executioner type must be "
                 "'Transient'.");
  }

  // Get end time
  Real end_time;
  if (isParamValid("end_time"))
    end_time = getParam<Real>("end_time");
  else
  {
    if (auto transient = dynamic_cast<TransientBase *>(_app.getExecutioner()))
      end_time = transient->endTime();
    else
      mooseError(
          "If the parameter 'end_time' is not provided, the executioner type must be 'Transient'.");
  }

  if (MooseUtils::absoluteFuzzyLessEqual(end_time, start_time))
    mooseError("The end time must be greater than the start time.");

  const auto time_interval = getParam<Real>("time_interval");
  const bool always_include_end_time = getParam<bool>("always_include_end_time");

  _times.push_back(start_time);
  while (true)
  {
    const auto proposed_new_time = _times.back() + time_interval;
    if (MooseUtils::absoluteFuzzyGreaterThan(proposed_new_time, end_time))
    {
      if (always_include_end_time && !MooseUtils::absoluteFuzzyEqual(_times.back(), end_time))
        _times.push_back(end_time);
      break;
    }
    else
      _times.push_back(proposed_new_time);
  }
}
