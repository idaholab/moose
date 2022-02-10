//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PIDControl.h"

registerMooseObject("ThermalHydraulicsApp", PIDControl);

InputParameters
PIDControl::validParams()
{
  InputParameters params = THMControl::validParams();
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
  : THMControl(parameters),
    _value(getControlData<Real>("input")),
    _set_point(getControlData<Real>("set_point")),
    _K_p(getParam<Real>("K_p")),
    _K_i(getParam<Real>("K_i")),
    _K_d(getParam<Real>("K_d")),
    _output(declareComponentControlData<Real>("output")),
    _initial_value(getParam<Real>("initial_value")),
    _integral(declareComponentControlData<Real>("integral")),
    _integral_old(getComponentControlDataOld<Real>("integral")),
    _error(declareComponentControlData<Real>("error")),
    _error_old(getComponentControlDataOld<Real>("error"))
{
  _integral = _initial_value;
}

void
PIDControl::execute()
{
  _error = _set_point - _value;
  _integral = _integral_old + _K_i * (_error * _dt);
  _output = _K_p * _error + _integral + _K_d * (_error - _error_old) / _dt;
}
