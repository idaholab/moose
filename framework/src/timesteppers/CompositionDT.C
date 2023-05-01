//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CompositionDT.h"
#include "MooseApp.h"
#include "Transient.h"

registerMooseObject("MooseApp", CompositionDT);

InputParameters
CompositionDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addParam<Real>("initial_dt", "Initial value of dt");
  params.addParam<std::vector<std::string>>(
      "lower_bound",
      "The input TimeSteppers to compose the lower bound time "
      "step size.  This can either be N timesteppers or 1 "
      "timestepper.");

  return params;
}

CompositionDT::CompositionDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _has_initial_dt(isParamValid("initial_dt")),
    _initial_dt(_has_initial_dt ? getParam<Real>("initial_dt") : 0.)
{
}

Real
CompositionDT::computeInitialDT()
{
  if (_has_initial_dt)
    return _initial_dt;
  else
    return computeDT();
}

Real
CompositionDT::computeDT()
{
  auto time_steppers = _app.getTimeStepperSystem().getTimeSteppers();

  if (time_steppers.empty())
    return 0;

  std::set<Real> dts;
  std::set<Real> bound_dt;

  auto input_bound = getParam<std::vector<std::string>>("lower_bound");
  std::set<std::string> lower_bound(input_bound.begin(), input_bound.end());

  for (auto const & [name, ptr] : time_steppers)
  {
    if (name != _app.getTimeStepperSystem().getFinalTimeStepperName())
    {
      mooseAssert(ptr, "Not exist");
      if (ptr->enabled())
      {
        ptr->computeStep();
        Real dt = ptr->getCurrentDT();
        if (!lower_bound.count(name))
          bound_dt.emplace(dt);
        else
          dts.emplace(dt);
      }
    }
  }

  _dt = produceCompositionDT(std::as_const(dts), std::as_const(bound_dt));

  return _dt;
}

Real
CompositionDT::getSequenceSteppers()
{
  auto time_sequence_steppers = _app.getTimeStepperSystem().getTimeSequenceSteppers();
  if (time_sequence_steppers.empty())
    return 0;
  Real hit_timestepper = std::numeric_limits<Real>::max();
  for (auto const & entry : time_sequence_steppers)
  {
    entry.second->init();
    Real next_time_to_hit = entry.second->getNextTimeInSequence();
    if (next_time_to_hit - _time <= _dt_min)
    {
      entry.second->increaseCurrentStep();
      next_time_to_hit = entry.second->getNextTimeInSequence();
    }
    if (hit_timestepper > next_time_to_hit)
      hit_timestepper = next_time_to_hit;
  }
  return hit_timestepper;
}

Real
CompositionDT::produceCompositionDT(const std::set<Real> & dts, const std::set<Real> & bound_dts)
{
  Real minDT, lower_bound = 0;
  if (!dts.empty())
    minDT = *dts.begin();
  if (!bound_dts.empty())
    lower_bound = *bound_dts.begin();

  auto ts = getSequenceSteppers();

  if (ts != 0)
    return std::min(std::abs(ts - _time), std::max(minDT, lower_bound));
  else
    return std::max(minDT, lower_bound);
}
