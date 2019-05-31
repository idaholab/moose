#include "TimeRampFunction.h"

registerMooseObject("THMApp", TimeRampFunction);

template <>
InputParameters
validParams<TimeRampFunction>()
{
  InputParameters params = validParams<Function>();

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
    _ramp_slope((_final_value - _initial_value) / _ramp_duration)
{
}

Real
TimeRampFunction::value(Real t, const Point & /*p*/) const
{
  if (t < _initial_time)
    mooseError("Time is less than this object's 'initial_time' parameter.");

  const Real elapsed_time = t - _initial_time;

  if (elapsed_time > _ramp_duration)
    return _final_value;
  else
    return _initial_value + _ramp_slope * elapsed_time;
}

RealVectorValue
TimeRampFunction::gradient(Real /*t*/, const Point & /*p*/) const
{
  mooseError("TimeRampFunction::gradient() is not implemented!");
}
