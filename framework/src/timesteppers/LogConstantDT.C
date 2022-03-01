//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LogConstantDT.h"

registerMooseObject("MooseApp", LogConstantDT);

InputParameters
LogConstantDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addClassDescription(
      "TimeStepper which imposes a time step constant in the logarithmic space");
  params.addRequiredRangeCheckedParam<Real>("log_dt", "log_dt > 0", "Time step in log10(time)");
  params.addRequiredRangeCheckedParam<Real>(
      "first_dt", "first_dt > 0", "Initial time step (in absolute time)");
  params.addRangeCheckedParam<Real>(
      "growth_factor",
      2,
      "growth_factor>=1",
      "Maximum ratio of new to previous timestep sizes following a step that required the time"
      " step to be cut due to a failed solve.");
  return params;
}

LogConstantDT::LogConstantDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _log_dt(getParam<Real>("log_dt")),
    _first_dt(getParam<Real>("first_dt")),
    _dt_factor(std::pow(10.0, _log_dt)),
    _growth_factor(getParam<Real>("growth_factor"))
{
}

Real
LogConstantDT::computeInitialDT()
{
  return _first_dt;
}

Real
LogConstantDT::computeDT()
{
  Real next = _time * _dt_factor;
  return std::min(next - _time, _growth_factor * getCurrentDT());
}
