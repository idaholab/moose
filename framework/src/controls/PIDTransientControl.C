//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PIDTransientControl.h"
#include "Function.h"
#include "Transient.h"

registerMooseObject("MooseApp", PIDTransientControl);

InputParameters
PIDTransientControl::validParams()
{
  InputParameters params = Control::validParams();
  params.addClassDescription(
      "Sets the value of a 'Real' input parameter (or postprocessor) based on a Proportional "
      "Integral Derivative control of a postprocessor to match a target a target value.");
  params.addRequiredParam<PostprocessorName>(
      "postprocessor", "The postprocessor to watch for controlling the specified parameter.");
  params.addRequiredParam<FunctionName>("target",
                                        "The target value 1D time function for the postprocessor");
  params.addRequiredParam<Real>("K_integral", "The coefficient multiplying the integral term");
  params.addRequiredParam<Real>("K_proportional",
                                "The coefficient multiplying the difference term");
  params.addRequiredParam<Real>("K_derivative", "The coefficient multiplying the derivative term");
  params.addParam<std::string>(
      "parameter",
      "The input parameter(s) to control. Specify a single parameter name and all "
      "parameters in all objects matching the name will be updated");
  params.addParam<std::string>("parameter_pp",
                               "The postprocessor to control. Should be accessed by reference by "
                               "the objects depending on its value.");
  params.addParam<Real>(
      "start_time", -std::numeric_limits<Real>::max(), "The time to start the PID controller at");
  params.addParam<Real>(
      "stop_time", std::numeric_limits<Real>::max(), "The time to stop the PID controller at");
  params.addParam<bool>(
      "reset_every_timestep",
      false,
      "Reset the PID integral when changing timestep, for coupling iterations within a timestep");
  params.addParam<bool>("reset_integral_windup",
                        true,
                        "Reset the PID integral when the error crosses zero and the integral is "
                        "larger than the error.");

  params.addParam<Real>("maximum_output_value",
                        std::numeric_limits<Real>::max(),
                        "Can be used to limit the maximum value output by the PID controller.");
  params.addParam<Real>("minimum_output_value",
                        -std::numeric_limits<Real>::max(),
                        "Can be used to limit the minimum value output by the PID controller.");
  params.addRangeCheckedParam<Real>(
      "maximum_change_rate",
      std::numeric_limits<Real>::max(),
      "maximum_change_rate>0",
      "Can be used to limit the absolute rate of change per second of value "
      "output by the PID controller.");
  return params;
}

PIDTransientControl::PIDTransientControl(const InputParameters & parameters)
  : Control(parameters),
    _current(getPostprocessorValueByName(getParam<PostprocessorName>("postprocessor"))),
    _target(getFunction("target")),
    _Kint(getParam<Real>("K_integral")),
    _Kpro(getParam<Real>("K_proportional")),
    _Kder(getParam<Real>("K_derivative")),
    _start_time(getParam<Real>("start_time")),
    _stop_time(getParam<Real>("stop_time")),
    _reset_every_timestep(getParam<bool>("reset_every_timestep")),
    _reset_integral_windup(getParam<bool>("reset_integral_windup")),
    _maximum_output_value(getParam<Real>("maximum_output_value")),
    _minimum_output_value(getParam<Real>("minimum_output_value")),
    _maximum_change_rate(getParam<Real>("maximum_change_rate")),
    _integral(0),
    _integral_old(0),
    _value_old(0),
    _t_step_old(-1),
    _old_delta(0)
{
  if (!_fe_problem.isTransient())
    mooseError("PIDTransientControl is only meant to be used when the problem is transient, for "
               "example with a Transient Executioner. Support for Steady "
               "Executioner can be added in the future, however certain parameters are currently "
               "not well defined for use with Picard iterations.");

  if (isParamValid("parameter") && isParamValid("parameter_pp"))
    paramError("parameter_pp",
               "Either a controllable parameter or a postprocessor to control should be specified, "
               "not both.");
  if (!isParamValid("parameter") && !isParamValid("parameter_pp"))
    mooseError("A parameter or a postprocessor to control should be specified.");
  if (isParamValid("parameter") && _reset_every_timestep)
    paramError(
        "reset_every_timestep",
        "Resetting the PID every time step is only supported using controlled postprocessors");
  if (_maximum_output_value <= _minimum_output_value)
    mooseError(
        "The parameters maximum_output_value and minimum_output_value are inconsistent. The value "
        "of maximum_output_value should be greater than the value of minimum_output_value.");
}

void
PIDTransientControl::execute()
{
  if (_t >= _start_time && _t < _stop_time)
  {
    // Get the current value of the controllable parameter
    Real value;
    if (isParamValid("parameter"))
      value = getControllableValue<Real>("parameter");
    else
      value = getPostprocessorValueByName(getParam<std::string>("parameter_pp"));

    // Save integral and controlled value at each time step
    // if the solver fails, a smaller time step will be used but _t_step is unchanged
    if (_t_step != _t_step_old)
    {
      // Reset the error integral if PID is only used within each timestep
      if (_reset_every_timestep)
        _integral = 0;

      _integral_old = _integral;
      _value_old = value;
      _t_step_old = _t_step;
      _old_delta = 0;
    }

    // If there were coupling/Picard iterations during the transient and they failed,
    // we need to reset the controlled value and the error integral to their initial value at the
    // beginning of the coupling process
    if (_app.getExecutioner()->fixedPointSolve().numFixedPointIts() == 1)
    {
      _integral = _integral_old;
      value = _value_old;
    }

    // Compute the delta between the current value of the postprocessor and the desired value
    Real delta = _current - _target.value(_t);

    // If desired, reset integral of the error if the error crosses zero
    if (_reset_integral_windup && delta * _old_delta < 0)
      _integral = 0;

    // Compute the three error terms and add them to the controlled value
    _integral += delta * _dt;
    value += _Kint * _integral + _Kpro * delta;
    if (_dt > 0)
      value += _Kder * delta / _dt;

    // If the maxmimum rate of change by the pid is fixed
    if (_maximum_change_rate != std::numeric_limits<Real>::max())
    {
      value = std::min(std::max(_value_old - _dt * _maximum_change_rate, value),
                       _value_old + _dt * _maximum_change_rate);
    }
    else
      value = std::min(std::max(_minimum_output_value, value), _maximum_output_value);

    // Set the new value of the postprocessor
    if (isParamValid("parameter"))
      setControllableValue<Real>("parameter", value);
    else
      _fe_problem.setPostprocessorValueByName(getParam<std::string>("parameter_pp"), value);

    // Keep track of the previous delta for integral windup control
    _old_delta = delta;
  }
}
