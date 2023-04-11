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

  params.addParam<std::vector<std::string>>(
      "input_timesteppers",
      "The input TimeSteppers to compose time step size.  This can either be N timesteppers or 1 "
      "timestepper.");

  params.addParam<MooseEnum>(
      "composition_type",
      getCompositionTypes(),
      "Provide a compose method to operate on input TimeSteppers. The composed time step size "
      "provide a max/min value for the current time step size. Avaliable methods includes max "
      "step, min step");

  params.addParam<std::string>("times_to_hit_timestepper",
                               "Provide user specified time to hit with a time sequence stepper");

  params.addParam<Real>("initial_dt", "Initial value of dt");

  params.addParamNamesToGroup("input_timesteppers composition_type",
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
  if (isParamValid("input_timesteppers"))
    _inputs = parameters.get<std::vector<std::string>>("input_timesteppers");

  if (isParamValid("times_to_hit_timestepper"))
    _hit_timestepper_name = parameters.get<std::string>("times_to_hit_timestepper");
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
  std::map<const std::string, Real> dts;

  for (const auto & name : _inputs)
  {
    auto time_stepper = getTimeStepper(name);
    Real dt = time_stepper->getCurrentDT();

    dts.emplace(name, dt);
  }

  auto base_stepper = getTimeStepper(_base_timestepper);
  _dt = produceCompositionDT(std::as_const(dts), base_stepper->getCurrentDT());

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

std::shared_ptr<TimeSequenceStepperBase>
CompositionDT::getSequenceStepper(const std::string & stpper_name)
{
  auto time_stepper = _app.getTimeStepperSystem().getTimeStepper(stpper_name);
  auto hit_timestepper = std::dynamic_pointer_cast<TimeSequenceStepperBase>(time_stepper);
  if (!hit_timestepper)
    mooseError("A TimeSequenceStepper type is required to generate times to hit");

  hit_timestepper->init();

  return hit_timestepper;
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
CompositionDT::produceCompositionDT(const std::map<const std::string, Real> & dts,
                                    const Real & base_dt)
{
  Real composeDT = 0;

  auto composition_type = getParam<MooseEnum>("composition_type");

  if (composition_type == "max")
    composeDT = std::min(maxTimeStep(dts), base_dt);

  else if (composition_type == "min")
    composeDT = std::max(minTimeStep(dts), base_dt);

  if (isParamValid("times_to_hit_timestepper"))
  {
    if (composeDT != 0)
      return produceHitDT(composeDT);
    else
      return produceHitDT(base_dt);
  }

  return composeDT;
}

Real
CompositionDT::produceHitDT(const Real & composeDT)
{
  auto ts = getSequenceStepper(_hit_timestepper_name);
  Real dt;

  Real next_time_to_hit = ts->getNextTimeInSequence();

  if (next_time_to_hit - _time <= _dt_min)
  {
    ts->increaseCurrentStep();
    Real next_time_to_hit = ts->getNextTimeInSequence();
    dt = std::min(next_time_to_hit - _time, composeDT);
  }
  else
  {
    dt = std::min(next_time_to_hit - _time, composeDT);
  }

  return dt;
}
