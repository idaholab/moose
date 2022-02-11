//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeRampFunction.h"

registerMooseObject("ThermalHydraulicsApp", TimeRampFunction);

InputParameters
TimeRampFunction::validParams()
{
  InputParameters params = Function::validParams();

  params.addRequiredParam<Real>("initial_value", "Initial value");
  params.addRequiredParam<Real>("final_value", "Final value");
  params.addRequiredParam<Real>("ramp_duration", "Duration, in seconds, of the ramp");
  params.addParam<Real>("initial_time", 0, "Initial time (necessary if not equal to zero)");

  params.addClassDescription("Ramps up to a value from another value over time.");

  return params;
}

TimeRampFunction::TimeRampFunction(const InputParameters & parameters)
  : Function(parameters),

    _initial_value(getParam<Real>("initial_value")),
    _final_value(getParam<Real>("final_value")),
    _ramp_duration(getParam<Real>("ramp_duration")),
    _initial_time(getParam<Real>("initial_time")),

    _ramp_end_time(_initial_time + _ramp_duration),
    _ramp_slope((_final_value - _initial_value) / _ramp_duration)
{
}

Real
TimeRampFunction::value(Real t, const Point & /*p*/) const
{
  const Real elapsed_time = t - _initial_time;

  if (t < _initial_time)
    return _initial_value;
  else if (t > _ramp_end_time)
    return _final_value;
  else
    return _initial_value + _ramp_slope * elapsed_time;
}

RealVectorValue
TimeRampFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError("TimeRampFunction::gradient() is not implemented!");
}
