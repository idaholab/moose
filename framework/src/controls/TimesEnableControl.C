//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimesEnableControl.h"
#include "Times.h"

registerMooseObject("MooseApp", TimesEnableControl);

InputParameters
TimesEnableControl::validParams()
{
  InputParameters params = ConditionalEnableControl::validParams();

  params.addRequiredParam<UserObjectName>(
      "times", "The Times object providing the list of times to turn on/off the objects.");

  params.addParam<Real>(
      "time_window",
      1e-8,
      "Window / tolerance on the absolute difference between the time step and the simulation");
  params.addParam<bool>(
      "act_on_time_stepping_across_a_time_point",
      true,
      "Whether to still perform the control action (enable/disable) if a time step went over a "
      "'time point' in the Times object without stopping near that exact time");

  params.addClassDescription(
      "Control for enabling/disabling objects when a certain time is reached.");

  return params;
}

TimesEnableControl::TimesEnableControl(const InputParameters & parameters)
  : ConditionalEnableControl(parameters),
    _times(getUserObject<Times>("times")),
    _time_window(getParam<Real>("time_window")),
    _act_on_time_stepping_across_time_point(
        getParam<bool>("act_on_time_stepping_across_a_time_point")),
    _prev_time_point_current(std::numeric_limits<Real>::max()),
    _prev_time_point(declareRestartableData<Real>("prev_time", std::numeric_limits<Real>::max())),
    _t_current(_fe_problem.time())
{
}

bool
TimesEnableControl::conditionMet(const unsigned int & /*i*/)
{
  // Retrieve time points around the current time
  const auto prev_time_point = _times.getPreviousTime(_t);
  const auto next_time_point = _times.getNextTime(_t, false);

  // Initialize the previous time point for the first time
  // By doing this here instead of the constructor, we avoid creating a construction dependency
  // between 'Times' and 'Controls'
  if (_prev_time_point == std::numeric_limits<Real>::max())
    _prev_time_point = _times.getTimeAtIndex(0);

  // Check if we are near a time point
  // We could have just missed the previous one or be right before the next one
  if (MooseUtils::absoluteFuzzyEqual(_t, prev_time_point, _time_window))
  {
    // Avoid always triggering on the next time step based on the prev_time_point value
    if (_act_on_time_stepping_across_time_point)
      _prev_time_point = next_time_point;
    return true;
  }
  else if (MooseUtils::absoluteFuzzyEqual(_t, next_time_point, _time_window))
  {
    if (_act_on_time_stepping_across_time_point)
      _prev_time_point = _times.getNextTime(next_time_point, false);
    return true;
  }

  // Update prev_time_point_current only if we changed time step since the last time conditionMet
  // was called
  if (_t != _t_current)
  {
    _prev_time_point_current = _prev_time_point;
    _t_current = _t;
  }

  // Check if we passed a time point
  if (_act_on_time_stepping_across_time_point && _t > _prev_time_point_current)
  {
    // We will need to pass the next time point next time to trigger the condition
    _prev_time_point = next_time_point;
    // we do not update prev_time_point_current in case conditionMet is called again in the same
    // time step
    return true;
  }

  return false;
}
