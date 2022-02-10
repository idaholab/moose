//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DelayControl.h"

registerMooseObject("ThermalHydraulicsApp", DelayControl);

InputParameters
DelayControl::validParams()
{
  InputParameters params = THMControl::validParams();
  params.addRequiredParam<std::string>("input", "The name of the control data that we read in.");
  params.addRequiredParam<Real>("tau", "Time period [s]");
  params.addParam<Real>("initial_value", 0., "Initial value");
  params.addClassDescription("Time delay control");
  return params;
}

DelayControl::DelayControl(const InputParameters & parameters)
  : THMControl(parameters),
    _initial_value(getParam<Real>("initial_value")),
    _input(getControlData<Real>("input")),
    _tau(getParam<Real>("tau")),
    _value(declareComponentControlData<Real>("value")),
    _input_time(declareRecoverableData<std::deque<Real>>("input_time")),
    _input_vals(declareRecoverableData<std::deque<Real>>("input_vals"))
{
  if (_tau < 0)
    mooseError("Negative values of 'tau' are not allowed.");

  addFnPoint(_t, _initial_value);
  _value = _initial_value;
}

void
DelayControl::addFnPoint(const Real & t, const Real & val)
{
  _input_time.push_back(t);
  _input_vals.push_back(val);
}

void
DelayControl::execute()
{
  _value = sampleFunction(_t - _tau);
  addFnPoint(_t, _input);

  // remove values that are beyond the time window
  while (_input_time[0] < (_t - _tau))
  {
    _input_time.pop_front();
    _input_vals.pop_front();
  }
}

Real
DelayControl::sampleFunction(const Real & t) const
{
  if (t <= _input_time.front())
    return _input_vals.front();
  if (t >= _input_time.back())
    return _input_vals.back();

  for (unsigned int i = 0; i + 1 < _input_time.size(); ++i)
    if (t >= _input_time[i] && t < _input_time[i + 1])
      return _input_vals[i] + (_input_vals[i + 1] - _input_vals[i]) * (t - _input_time[i]) /
                                  (_input_time[i + 1] - _input_time[i]);

  throw std::out_of_range("Unreachable");
  return 0;
}
