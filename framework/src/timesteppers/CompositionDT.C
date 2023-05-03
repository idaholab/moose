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

#include <limits>

registerMooseObject("MooseApp", CompositionDT);

InputParameters
CompositionDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addParam<Real>("initial_dt", "Initial value of dt");
  params.addParam<std::vector<std::string>>(
      "lower_bound",
      "The maximum of these TimeSteppers will form the lower bound on the time "
      "step size. A single or multiple time steppers may be specified.");
  params.addClassDescription("The time stepper take all the other time steppers as input and "
                             "return the minimum time step size.");

  return params;
}

CompositionDT::CompositionDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _has_initial_dt(isParamValid("initial_dt")),
    _initial_dt(_has_initial_dt ? getParam<Real>("initial_dt") : 0.),
    _lower_bound(getParam<std::vector<std::string>>("lower_bound"))
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

  std::set<std::string> lower_bound(_lower_bound.begin(), _lower_bound.end());

  for (auto const & [name, ptr] : time_steppers)
  {
    if (name != _app.getTimeStepperSystem().getFinalTimeStepperName())
    {
      mooseAssert(ptr, "Not exist");
      if (ptr->enabled())
      {
        ptr->computeStep();
        Real dt = ptr->getCurrentDT();
        if (lower_bound.count(name))
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
CompositionDT::getSequenceSteppersNextTime()
{
  auto time_sequence_steppers = _app.getTimeStepperSystem().getTimeSequenceSteppers();
  if (time_sequence_steppers.empty())
    return 0;
  Real next_time_to_hit = std::numeric_limits<Real>::max();
  for (auto const & entry : time_sequence_steppers)
  {
    entry.second->init();
    Real ts_time_to_hit = entry.second->getNextTimeInSequence();
    if (ts_time_to_hit - _time <= _dt_min)
    {
      entry.second->increaseCurrentStep();
      ts_time_to_hit = entry.second->getNextTimeInSequence();
    }
    if (next_time_to_hit > ts_time_to_hit)
      next_time_to_hit = ts_time_to_hit;
  }
  return next_time_to_hit;
}

Real
CompositionDT::produceCompositionDT(const std::set<Real> & dts, const std::set<Real> & bound_dts)
{
  Real minDT, lower_bound = 0;
  if (!dts.empty())
    minDT = *dts.begin();
  if (!bound_dts.empty())
    lower_bound = *bound_dts.rbegin();

  auto ts = getSequenceSteppersNextTime();

  if (ts != 0)
    return std::min((ts - _time), std::max(minDT, lower_bound));
  else
    return std::max(minDT, lower_bound);
}
