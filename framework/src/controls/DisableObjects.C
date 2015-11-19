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
#include "DisableObjects.h"
#include "Function.h"
#include "Transient.h"
#include "MooseUtils.h"

template<>
InputParameters validParams<DisableObjects>()
{
  InputParameters params = validParams<Control>();
  params.addRequiredParam<std::vector<std::string> >("disable", "A list of object tags to disable.");
  params.addParam<std::vector<Real> >("start_time", "The time at which the objects are to be disabled.");
  params.addParam<std::vector<Real> >("end_time", "The time at which the objects are to be re-enabled.");
  return params;
}


DisableObjects::DisableObjects(const InputParameters & parameters) :
    Control(parameters),
    _disable(getParam<std::vector<std::string> >("disable"))
{
  // Error if not a transient problem
  if (!_fe_problem.isTransient())
    mooseError("DisableObjects objects only operate on transient problems.");

  // Set start time
  if (isParamValid("start_time"))
    _start_time = getParam<std::vector<Real> >("start_time");
  else
    _start_time = std::vector<Real>(_disable.size(), _app.executioner()->getParam<Real>("start_time"));

  // Set end time
  if (isParamValid("end_time"))
    _end_time = getParam<std::vector<Real> >("end_time");
  else
    _end_time = std::vector<Real>(_disable.size(), std::numeric_limits<Real>::max());

  // Check that start/end time are the same length
  if (_end_time.size() != _start_time.size())
    mooseError("The end time and start time vectors must be the same length.");

  // Resize the start/end times if only a single value given
  if (_end_time.size() == 1 && _disable.size() > 1)
  {
    _end_time = std::vector<Real>(_disable.size(), _end_time[0]);
    _start_time = std::vector<Real>(_disable.size(), _start_time[0]);
  }
  else if (_end_time.size() != _disable.size())
    mooseError("The start/end time input must be a scalar or the same length as the disable list.");
}


void
DisableObjects::execute()
{
  for (unsigned int i = 0; i < _disable.size(); ++i)
  {
    // If the current time falls between the start and end time, disable the object (_t >= _start_time and _t <= _end_time)
    if (MooseUtils::absoluteFuzzyGreaterEqual(_t, _start_time[i]) && MooseUtils::absoluteFuzzyLessEqual(_t, _end_time[i]))
      setControllableValueByName<bool>(_disable[i], std::string("enable"), false);

    else
      setControllableValueByName<bool>(_disable[i], std::string("enable"), true);

  }
}
