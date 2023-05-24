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
#include "IterationAdaptiveDT.h"

#include <limits>

registerMooseObject("MooseApp", CompositionDT);

InputParameters
CompositionDT::compositionDTParams()
{
  auto params = emptyInputParameters();

  params.addParam<Real>("initial_dt", "Initial value of dt");
  params.addParam<std::vector<std::string>>(
      "lower_bound",
      "The maximum of these TimeSteppers will form the lower bound on the time "
      "step size. A single or multiple time steppers may be specified.");

  return params;
}

InputParameters
CompositionDT::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params += CompositionDT::compositionDTParams();

  params.addClassDescription("The time stepper take all the other time steppers as input and "
                             "return the minimum time step size.");

  return params;
}

CompositionDT::CompositionDT(const InputParameters & parameters)
  : TimeStepper(parameters),
    _has_initial_dt(isParamValid("initial_dt")),
    _initial_dt(_has_initial_dt ? getParam<Real>("initial_dt") : 0.),
    _lower_bound(getParam<std::vector<std::string>>("lower_bound").begin(),
                 getParam<std::vector<std::string>>("lower_bound").end()),
    _last_time_stepper(nullptr),
    _bound_last_time_stepper(nullptr),
    _last_time_sequence_stepper(nullptr)
{
  // Make sure the steppers in "lower_bound" exist
  const auto time_steppers = getTimeSteppers();
  for (const auto & time_stepper_name : _lower_bound)
    if (std::find_if(time_steppers.begin(),
                     time_steppers.end(),
                     [&time_stepper_name](const auto & ts)
                     { return ts->name() == time_stepper_name; }) == time_steppers.end())
      paramError(
          "lower_bound", "Failed to find a timestepper with the name '", time_stepper_name, "'");
}

void
CompositionDT::init()
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->init();
}
void
CompositionDT::preExecute()
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->preExecute();
}

void
CompositionDT::preSolve()
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->preSolve();
}

void
CompositionDT::postSolve()
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->postSolve();
}

void
CompositionDT::postExecute()
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->postExecute();
}

void
CompositionDT::preStep()
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->preStep();
}

void
CompositionDT::postStep()
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->postStep();
}

bool
CompositionDT::constrainStep(Real & dt)
{
  bool at_sync_point = TimeStepper::constrainStep(dt);
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (!ts->constrainStep(dt))
      return false;
  return at_sync_point;
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
  const auto time_steppers = getTimeSteppers();
  // Note : compositionDT requires other active time steppers as input so no active time steppers
  // or only compositionDT is active is not allowed
  if (time_steppers.size() < 2 && time_steppers[0] == this)
    mooseError("No TimeStepper(s) are currently active to compute a timestep");

  std::set<Real> dts;
  std::set<Real> bound_dt;

  for (auto & ts : time_steppers)
    if (ts != this && !dynamic_cast<TimeSequenceStepperBase *>(ts))
    {
      ts->computeStep();
      const auto dt = ts->getCurrentDT();

      if (_lower_bound.count(ts->name()))
      {
        bound_dt.emplace(dt);
        if (bound_dt.find(dt) == (--bound_dt.end()))
          _bound_last_time_stepper = ts;
      }
      else
      {
        dts.emplace(dt);
        if (dts.find(dt) == dts.begin())
          _last_time_stepper = ts;
      }
    }

  _dt = produceCompositionDT(std::as_const(dts), std::as_const(bound_dt));

  return _dt;
}

Real
CompositionDT::getSequenceSteppersNextTime()
{
  const auto time_steppers = getTimeSteppers();

  std::vector<TimeSequenceStepperBase *> time_sequence_steppers;
  for (auto & ts : time_steppers)
    if (auto tss = dynamic_cast<TimeSequenceStepperBase *>(ts))
      time_sequence_steppers.push_back(tss);

  if (time_sequence_steppers.empty())
    return 0;

  Real next_time_to_hit = std::numeric_limits<Real>::max();
  for (auto & tss : time_sequence_steppers)
  {
    Real ts_time_to_hit = tss->getNextTimeInSequence();
    if (ts_time_to_hit - _time <= _dt_min)
    {
      tss->increaseCurrentStep();
      ts_time_to_hit = tss->getNextTimeInSequence();
    }
    if (next_time_to_hit > ts_time_to_hit)
    {
      _last_time_sequence_stepper = tss;
      next_time_to_hit = ts_time_to_hit;
    }
  }
  return next_time_to_hit;
}

Real
CompositionDT::produceCompositionDT(const std::set<Real> & dts, const std::set<Real> & bound_dts)
{
  Real minDT, lower_bound, dt;
  minDT = lower_bound = dt = 0.0;
  if (!dts.empty())
    minDT = *dts.begin();
  if (!bound_dts.empty())
    lower_bound = *bound_dts.rbegin();

  if (minDT > lower_bound)
    dt = minDT;
  else
  {
    dt = lower_bound;
    _last_time_stepper = _bound_last_time_stepper;
  }

  auto ts = getSequenceSteppersNextTime();

  if (ts != 0 && (ts - _time) < dt)
  {
    _last_time_stepper = _last_time_sequence_stepper;

    return std::min((ts - _time), dt);
  }
  else
    return dt;
}

std::vector<TimeStepper *>
CompositionDT::getTimeSteppers() const
{
  std::vector<TimeStepper *> time_steppers;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("TimeStepper")
      .queryInto(time_steppers);
  return time_steppers;
}

void
CompositionDT::step()
{
  if (_last_time_stepper)
    _last_time_stepper->step();
  else
    TimeStepper::step();
}

void
CompositionDT::acceptStep()
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->acceptStep();
}

void
CompositionDT::rejectStep()
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->rejectStep();
}

bool
CompositionDT::converged() const
{
  if (_last_time_stepper)
    return _last_time_stepper->converged();
  return TimeStepper::converged();
}

void
CompositionDT::forceTimeStep(Real dt)
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->forceTimeStep(dt);
}

void
CompositionDT::forceNumSteps(const unsigned int num_steps)
{
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts != this)
      ts->forceNumSteps(num_steps);
}
