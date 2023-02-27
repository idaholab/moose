//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimePeriodBase.h"
#include "Function.h"
#include "Transient.h"
#include "MooseUtils.h"

InputParameters
TimePeriodBase::validParams()
{
  InputParameters params = ConditionalEnableControl::validParams();
  params.addClassDescription(
      "Base class for controlling the enabled/disabled state of objects with time.");
  return params;
}

TimePeriodBase::TimePeriodBase(const InputParameters & parameters)
  : ConditionalEnableControl(parameters)
{
  // Error if not a transient problem
  if (!_fe_problem.isTransient())
    mooseError("TimePeriodBase objects only operate on transient problems.");
}

void
TimePeriodBase::setupTimes()
{
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
