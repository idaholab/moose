/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

// MOOSE includes
#include "TimePeriod.h"
#include "Function.h"
#include "Transient.h"
#include "MooseUtils.h"

template <>
InputParameters
validParams<TimePeriod>()
{
  InputParameters params = validParams<Control>();
  params.addParam<std::vector<std::string>>(
      "disable_objects", std::vector<std::string>(), "A list of object tags to disable.");
  params.addParam<std::vector<std::string>>(
      "enable_objects", std::vector<std::string>(), "A list of object tags to enable.");
  params.addParam<std::vector<Real>>("start_time",
                                     "The time at which the objects are to be enabled/disabled.");
  params.addParam<std::vector<Real>>("end_time",
                                     "The time at which the objects are to be enable/disabled.");
  params.addParam<bool>(
      "set_sync_times", false, "Set the start and end time as execute sync times.");
  params.addParam<bool>("set_outside_of_range",
                        true,
                        "When true the disable/enable lists are set "
                        "to opposite values when outside of the "
                        "given time range.");
  return params;
}

TimePeriod::TimePeriod(const InputParameters & parameters)
  : Control(parameters),
    _enable(getParam<std::vector<std::string>>("enable_objects")),
    _disable(getParam<std::vector<std::string>>("disable_objects")),
    _set_outside_of_range(getParam<bool>("set_outside_of_range"))
{
  // Error if not a transient problem
  if (!_fe_problem.isTransient())
    mooseError("TimePeriod objects only operate on transient problems.");

  // Error if enable and disable lists are both empty
  if (_enable.empty() && _disable.empty())
    mooseError(
        "Either or both of the 'enable_objects' and 'disable_objects' parameters must be set.");

  // Set start time
  if (isParamValid("start_time"))
    _start_time = getParam<std::vector<Real>>("start_time");
  else
    _start_time = {_app.executioner()->getParam<Real>("start_time")};

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
TimePeriod::execute()
{
  // ENABLE
  for (auto i = beginIndex(_enable); i < _enable.size(); ++i)
  {
    // If the current time falls between the start and end time, ENABLE the object (_t >=
    // _start_time and _t < _end_time)
    if (MooseUtils::absoluteFuzzyGreaterEqual(_t, _start_time[i]) &&
        MooseUtils::absoluteFuzzyLessThan(_t, _end_time[i]))
      setControllableValueByName<bool>(_enable[i], std::string("enable"), true);

    else if (_set_outside_of_range)
      setControllableValueByName<bool>(_enable[i], std::string("enable"), false);
  }

  // DISABLE
  for (auto i = beginIndex(_disable); i < _disable.size(); ++i)
  {
    // If the current time falls between the start and end time, DISABLE the object (_t >=
    // _start_time and _t < _end_time)
    if (MooseUtils::absoluteFuzzyGreaterEqual(_t, _start_time[i]) &&
        MooseUtils::absoluteFuzzyLessThan(_t, _end_time[i]))
      setControllableValueByName<bool>(_disable[i], std::string("enable"), false);

    else if (_set_outside_of_range)
      setControllableValueByName<bool>(_disable[i], std::string("enable"), true);
  }
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
