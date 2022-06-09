//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionDT.h"
#include "Function.h"
#include "PiecewiseBase.h"
#include <limits>

registerMooseObject("MooseApp", FunctionDT);

InputParameters
FunctionDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addRequiredParam<FunctionName>(
      "function", "The name of the time-dependent function that prescribes the time step size.");
  params.addParam<Real>("growth_factor",
                        std::numeric_limits<Real>::max(),
                        "Maximum ratio of new to previous timestep sizes.");
  params.addParam<Real>("min_dt", 0, "The minimal dt to take.");
  params.addClassDescription(
      "Timestepper whose steps vary over time according to a user-defined function");

  return params;
}

FunctionDT::FunctionDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    FunctionInterface(this),
    _function(getFunction("function")),
    _growth_factor(getParam<Real>("growth_factor")),
    _min_dt(getParam<Real>("min_dt"))
{

  // If dt is given by piece-wise linear and constant function, we add the domain into
  // _time_knots, so that the time stepper hits those time points
  if (const PiecewiseBase * pw = dynamic_cast<const PiecewiseBase *>(&_function))
  {
    unsigned int n_knots = pw->functionSize();
    for (unsigned int i = 0; i < n_knots; i++)
      _time_knots.push_back(pw->domain(i));
  }
}

void
FunctionDT::init()
{
  removeOldKnots();
}

void
FunctionDT::removeOldKnots()
{
  while ((_time_knots.size() > 0) &&
         (*_time_knots.begin() <= _time || std::abs(*_time_knots.begin() - _time) < 1e-10))
    _time_knots.erase(_time_knots.begin());
}

Real
FunctionDT::computeInitialDT()
{
  return computeDT();
}

Real
FunctionDT::computeDT()
{
  Real local_dt = _function.value(_time, _point_zero);
  ;

  // sync to time knot
  if ((_time_knots.size() > 0) && (_time + local_dt >= (*_time_knots.begin())))
    local_dt = (*_time_knots.begin()) - _time;
  // honor minimal dt
  if (local_dt < _min_dt)
    local_dt = _min_dt;

  if ((local_dt > (_dt * _growth_factor)) && _dt > 0)
    local_dt = _dt * _growth_factor;

  return local_dt;
}

void
FunctionDT::postStep()
{
  removeOldKnots();
}
