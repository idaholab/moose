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

#include "LogConstantDT.h"

template <>
InputParameters
validParams<LogConstantDT>()
{
  InputParameters params = validParams<TimeStepper>();
  params.addClassDescription("TimeStepper which imposes a time step constant in the logarithmic space");
  params.addRequiredParam<Real>("dt", "Time step in log10(time)");
  params.addRequiredParam<Real>("first_dt", "Initial time step (in absolute time)");
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
    _log_dt(getParam<Real>("dt")),
    _first_dt(getParam<Real>("first_dt")),
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
  Real next = std::pow(10., std::log10(_time) + _log_dt);
  return std::min(next - _time, _growth_factor * getCurrentDT());
}
