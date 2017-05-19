#include "PIDControl.h"

template <>
InputParameters
validParams<PIDControl>()
{
  InputParameters params = validParams<RELAP7Control>();
  params.addRequiredParam<std::string>("input", "The name of the control data that we read in.");
  params.addRequiredParam<std::string>("set_point",
                                       "The name of the control data with the set point.");
  params.addRequiredParam<Real>("initial_value", "The initial value for the integral part.");
  params.addRequiredParam<Real>("K_p", "The coefficient for the proportional term.");
  params.addRequiredParam<Real>("K_i", "The coefficient for the integral term.");
  params.addRequiredParam<Real>("K_d", "The coefficient for the derivative term.");
  return params;
}

PIDControl::PIDControl(const InputParameters & parameters)
  : RELAP7Control(parameters),
    _value(getControlData<Real>("input")),
    _set_point(getControlData<Real>("set_point")),
    _K_p(getParam<Real>("K_p")),
    _K_i(getParam<Real>("K_i")),
    _K_d(getParam<Real>("K_d")),
    _output(declareControlData<Real>("output")),
    _integral(declareRestartableData<Real>("integral", getParam<Real>("initial_value"))),
    _error_old(0.)
{
  if (_K_d > 0)
    mooseWarning("You are trying to use the derivative term in your PID controller. That will have "
                 "issues if your solve fails to converge, because the proper recover from failed "
                 "solves has not been implemented yet.");
}

void
PIDControl::execute()
{
  Real error = _set_point - _value;
  _integral = _integral + _K_i * (error * _dt);
  _output = _K_p * error + _integral + _K_d * (error - _error_old) / _dt;
  // FIXME: if time step solve fails, _error_old will be wrong. This needs to happen only on a
  // successful solve.
  _error_old = error;
}
