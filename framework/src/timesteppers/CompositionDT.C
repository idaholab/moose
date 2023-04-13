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

  std::map<const std::string, Real> dts;
  for (auto const & [name, ptr] : time_steppers)
  {
    if (name != _app.getTimeStepperSystem().getFinalTimeStepperName())
    {
      mooseAssert(ptr, "Not exist");
      ptr->computeStep();
      Real dt = ptr->getCurrentDT();
      dts.emplace(name, dt);
    }
  }

  _dt = produceCompositionDT(std::as_const(dts));

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
CompositionDT::minTimeStep(const std::map<const std::string, Real> & dts)
{
  auto minDT = std::min_element(
      dts.begin(), dts.end(), [](const auto & a, const auto & b) { return a.second < b.second; });
  return minDT->second;
}

Real
CompositionDT::produceCompositionDT(const std::map<const std::string, Real> & dts)
{
  Real minDT = 0;

  minDT = minTimeStep(dts);

  auto ts = getSequenceSteppers();

  if (ts != 0)
    return std::min(std::abs(ts - _time), minDT);
  else
    return minDT;
}
