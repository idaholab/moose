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
  params.addParam<Real>("start_time", "The time at which the objects are to be disabled.");
  params.addParam<Real>("end_time", "The time at which the objects are to be re-enabled.");
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
    _start_time = getParam<Real>("start_time");
  else
    _start_time = _app.executioner()->getParam<Real>("start_time");

  // Set end time
  if (isParamValid("end_time"))
    _end_time = getParam<Real>("end_time");
  else
    _end_time = std::numeric_limits<Real>::max();
}


void
DisableObjects::execute()
{
  // If the current time falls between the start and end time, disable the object (_t >= _start_time and _t <= _end_time)
  if (MooseUtils::absoluteFuzzyGreaterEqual(_t, _start_time) && MooseUtils::absoluteFuzzyLessEqual(_t, _end_time))
    for (std::vector<std::string>::const_iterator it = _disable.begin(); it != _disable.end(); ++it)
      setControllableValueByName<bool>(*it, std::string("enable"), false);

  else
    for (std::vector<std::string>::const_iterator it = _disable.begin(); it != _disable.end(); ++it)
      setControllableValueByName<bool>(*it, std::string("enable"), true);
}
