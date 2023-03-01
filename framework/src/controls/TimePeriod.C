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

InputParameters
TimePeriod::validParams()
{
  InputParameters params = TimePeriodBase::validParams();

  params.addClassDescription("Control the enabled/disabled state of objects with time.");

  params.addParam<std::vector<Real>>("start_time",
                                     "The time at which the objects are to be enabled/disabled.");
  params.addParam<std::vector<Real>>("end_time",
                                     "The time at which the objects are to be enable/disabled.");
  params.addParam<bool>(
      "set_sync_times", false, "Set the start and end time as execute sync times.");

  return params;
}

TimePeriod::TimePeriod(const InputParameters & parameters) : TimePeriodBase(parameters)
{
  // Set start time
  if (isParamValid("start_time"))
    _start_time = getParam<std::vector<Real>>("start_time");
  else
    _start_time = {_app.getExecutioner()->getParam<Real>("start_time")};

  // Set end time
  if (isParamValid("end_time"))
    _end_time = getParam<std::vector<Real>>("end_time");
  else
    _end_time = std::vector<Real>(_start_time.size(), std::numeric_limits<Real>::max());

  // Call base method to populate control times.
  TimePeriodBase::setupTimes();
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
