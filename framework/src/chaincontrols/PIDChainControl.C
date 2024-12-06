//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PIDChainControl.h"
#include "MooseUtils.h"

registerMooseObject("MooseApp", PIDChainControl);

InputParameters
PIDChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();

  params.addClassDescription("Implements a proportional-integral-derivative (PID) controller.");

  params.addRequiredParam<std::string>("input", "Input control data");
  params.addRequiredParam<std::string>("set_point", "Set point control data");
  params.addParam<Real>("initial_integral", 0.0, "Initial value for the integral component");
  params.addRequiredParam<Real>("K_p", "Coefficient for the proportional term");
  params.addRequiredParam<Real>("K_i", "Coefficient for the integral term");
  params.addRequiredParam<Real>("K_d", "Coefficient for the derivative term");

  return params;
}

PIDChainControl::PIDChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    _input(getChainControlData<Real>("input")),
    _set_point(getChainControlData<Real>("set_point")),
    _K_p(getParam<Real>("K_p")),
    _K_i(getParam<Real>("K_i")),
    _K_d(getParam<Real>("K_d")),
    _error(declareChainControlData<Real>("error")),
    _error_old(getChainControlDataOldByName<Real>(fullControlDataName("error"))),
    _proportional(declareChainControlData<Real>("proportional")),
    _integral(declareChainControlData<Real>("integral")),
    _integral_old(getChainControlDataOldByName<Real>(fullControlDataName("integral"))),
    _derivative(declareChainControlData<Real>("derivative")),
    _output(declareChainControlData<Real>("value")),
    _previous_time(declareRestartableData<Real>("previous_time"))
{
  _integral = getParam<Real>("initial_integral");
  _previous_time = std::numeric_limits<Real>::max();
}

void
PIDChainControl::execute()
{
  if (!MooseUtils::absoluteFuzzyEqual(_t, _previous_time))
    updateValues();

  _previous_time = _t;
}

void
PIDChainControl::updateValues()
{
  _error = _set_point - _input;

  _proportional = _K_p * _error;
  _integral = _integral_old + _K_i * (_error * _dt);
  if (MooseUtils::absoluteFuzzyEqual(_dt, 0.0))
    _derivative = 0.0;
  else
    _derivative = _K_d * (_error - _error_old) / _dt;

  _output = _proportional + _integral + _derivative;
}
