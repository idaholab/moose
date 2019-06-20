//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "TimePeriod.h"
#include "Function.h"
#include "Transient.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", TimePeriod);

template <>
InputParameters
validParams<TimePeriod>()
{
  InputParameters params = validParams<ConditionalEnableControl>();

  params.addClassDescription("Control the enabled/disabled state of objects with time.");

  params.addParam<std::vector<Real>>("start_time",
                                     "The time at which the objects are to be enabled/disabled.");
  params.addParam<std::vector<Real>>("end_time",
                                     "The time at which the objects are to be enable/disabled.");
  params.addParam<bool>(
      "set_sync_times", false, "Set the start and end time as execute sync times.");

  return params;
}

TimePeriod::TimePeriod(const InputParameters & parameters) : ConditionalEnableControl(parameters)
{
  // Error if not a transient problem
  if (!_fe_problem.isTransient())
    mooseError("TimePeriod objects only operate on transient problems.");

  // Set start time
  if (isParamValid("start_time"))
    _start_time = getParam<std::vector<Real>>("start_time");
  else
    _start_time = {_app.getExecutioner()->getParamTempl<Real>("start_time")};

  // Set end time
  if (isParamValid("end_time"))
    _end_time = getParam<std::vector<Real>>("end_time");
  else
    _end_time = std::vector<Real>(_start_time.size(), std::numeric_limits<Real>::max());

  // Check that start/end time are the same length
  if (_end_time.size() != _start_time.size())
    mooseError("The end time and start time vectors must be the same length.");

  // Resize the start/end times if only a single value given
  if (_end_time.size() == 1 && (_disable.size() > 1 || _enable.size() > 1))
  {
    unsigned int size = std::max(_disable.size(), _enable.size());
    _end_time = std::vector<Real>(size, _end_time[0]);
    _start_time = std::vector<Real>(size, _start_time[0]);
  }
  else if (_end_time.size() != _disable.size() && _end_time.size() != _enable.size())
    mooseError("The start/end time input must be a scalar or the same length as the enable/disable "
               "lists.");

  // Test that start and end times are in proper order
  for (unsigned int i = 0; i < _start_time.size(); ++i)
    if (_start_time[i] >= _end_time[i])
      mooseError("The start time(s) must be less than the end time(s).");
}

void
TimePeriod::initialSetup()
{
  if (getParam<bool>("set_sync_times"))
  {
    std::set<Real> & sync_times = _app.getOutputWarehouse().getSyncTimes();
    sync_times.insert(_start_time.begin(), _start_time.end());
    sync_times.insert(_end_time.begin(), _end_time.end());
  }
}

bool
TimePeriod::conditionMet(const unsigned int & i)
{
  return MooseUtils::absoluteFuzzyGreaterEqual(_t, _start_time[i]) &&
         MooseUtils::absoluteFuzzyLessThan(_t, _end_time[i]);
}
