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
#include "TimeSequenceStepperBase.h"

registerMooseObject("MooseApp", CompositionDT);

InputParameters
CompositionDT::validParams()
{
  InputParameters params = TimeStepper::validParams();

  params.addRequiredParam<std::string>(
      "base_timestepper", "Provide a time stepper type to produce a base time step size");

  params.addParam<std::vector<std::string>>("maximum_step_from",
                                            "The input TimeSteppers to compose the maximum time "
                                            "step size.  This can either be N timesteppers or 1 "
                                            "timestepper.");

  params.addParam<std::vector<std::string>>("minimum_step_from",
                                            "The input TimeSteppers to compose the minimum time "
                                            "step size.  This can either be N timesteppers or 1 "
                                            "timestepper.");

  params.addParam<std::vector<std::string>>(
      "times_to_hit_timestepper",
      "Provide user specified time to hit with a time sequence stepper");

  params.addParam<Real>("initial_dt", "Initial value of dt");

  params.addParamNamesToGroup("maximum_step_from minimum_step_from",
                              "Time Steppers for composition");

  params.addParamNamesToGroup("times_to_hit_timestepper", "Time Sequence Stepper");

  params.addClassDescription("Compose multiple TimeSteppers together to generate time step size.");

  return params;
}

CompositionDT::CompositionDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _has_initial_dt(isParamValid("initial_dt")),
    _initial_dt(_has_initial_dt ? getParam<Real>("initial_dt") : 0.),
    _base_timestepper(getParam<std::string>("base_timestepper"))
{
  if (isParamValid("maximum_step_from"))
    _maximum_step_inputs = parameters.get<std::vector<std::string>>("maximum_step_from");

  if (isParamValid("minimum_step_from"))
    _minimum_step_inputs = parameters.get<std::vector<std::string>>("minimum_step_from");

  if (isParamValid("times_to_hit_timestepper"))
    _hit_timestepper_name = parameters.get<std::vector<std::string>>("times_to_hit_timestepper");
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
  std::map<const std::string, Real> max_dts;
  std::map<const std::string, Real> min_dts;

  for (const auto & name : _maximum_step_inputs)
  {
    auto time_stepper = getTimeStepper(name);
    Real dt = time_stepper->getCurrentDT();

    max_dts.emplace(name, dt);
  }

  for (const auto & name : _minimum_step_inputs)
  {
    auto time_stepper = getTimeStepper(name);
    Real dt = time_stepper->getCurrentDT();

    min_dts.emplace(name, dt);
  }

  auto base_step = getTimeStepper(_base_timestepper)->getCurrentDT();
  Real max_dt = _dt_max;
  Real mix_dt = _dt_min;
  if (!max_dts.empty())
    max_dt = maxTimeStep(std::as_const(max_dts));
  if (!min_dts.empty())
    mix_dt = minTimeStep(std::as_const(min_dts));

  _dt = produceCompositionDT(max_dt, mix_dt, base_step);

  return _dt;
}

std::shared_ptr<TimeStepper>
CompositionDT::getTimeStepper(const std::string & stpper_name)
{
  auto time_stepper = _app.getTimeStepperSystem().getTimeStepper(stpper_name);
  mooseAssert(time_stepper, "Not exist");
  time_stepper->computeStep();

  return time_stepper;
}

Real
CompositionDT::getSequenceSteppers()
{

  Real hit_time = std::numeric_limits<Real>::max();
  for (const auto & name : _hit_timestepper_name)
  {
    auto ts_ptr = _app.getTimeStepperSystem().getTimeStepper(name);
    auto hit_stepper = std::dynamic_pointer_cast<TimeSequenceStepperBase>(ts_ptr);
    if (!hit_stepper)
      mooseError("A TimeSequenceStepper type is required to generate times to hit");
    hit_stepper->init();
    Real next_time_to_hit = hit_stepper->getNextTimeInSequence();
    if (next_time_to_hit - _time <= _dt_min)
    {
      hit_stepper->increaseCurrentStep();
      next_time_to_hit = hit_stepper->getNextTimeInSequence();
    }
    if (hit_time > next_time_to_hit)
      hit_time = next_time_to_hit;
  }
  return hit_time;
}

Real
CompositionDT::maxTimeStep(const std::map<const std::string, Real> & dts)
{
  auto maxDT = std::max_element(
      dts.begin(), dts.end(), [](const auto & x, const auto & y) { return x.second < y.second; });

  return maxDT->second;
}

Real
CompositionDT::minTimeStep(const std::map<const std::string, Real> & dts)
{
  auto minDT = std::min_element(
      dts.begin(), dts.end(), [](const auto & a, const auto & b) { return a.second < b.second; });
  return minDT->second;
}

Real
CompositionDT::produceCompositionDT(const Real & max_dt, const Real & min_dt, const Real & base_dt)
{
  Real ts = getSequenceSteppers();
  Real hit_dt = ts - _time;
  Real composed_dt = 0.0;

  // min_dt <composed_dt < max_dt
  composed_dt = std::min(std::max(base_dt, min_dt), max_dt);

  // if there is a time to hit, honor that time point
  if (ts != 0)
    composed_dt = std::min(hit_dt, composed_dt);

  return composed_dt;
}
