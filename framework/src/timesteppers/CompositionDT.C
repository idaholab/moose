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
    _current_time_stepper(nullptr),
    _largest_bound_time_stepper(nullptr),
    _closest_time_sequence_stepper(nullptr)
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

template <typename Lambda>
void
CompositionDT::actOnTimeSteppers(Lambda && act)
{
  for (auto & ts : getTimeSteppers())
    act(*ts);
}

void
CompositionDT::init()
{
  actOnTimeSteppers([](auto & ts) { ts.init(); });
}

void
CompositionDT::preExecute()
{
  actOnTimeSteppers([](auto & ts) { ts.preExecute(); });
}

void
CompositionDT::preSolve()
{
  actOnTimeSteppers([](auto & ts) { ts.preSolve(); });
}

void
CompositionDT::postSolve()
{
  actOnTimeSteppers([](auto & ts) { ts.postSolve(); });
}

void
CompositionDT::postExecute()
{
  actOnTimeSteppers([](auto & ts) { ts.postExecute(); });
}

void
CompositionDT::preStep()
{
  actOnTimeSteppers([](auto & ts) { ts.preStep(); });
}

void
CompositionDT::postStep()
{
  actOnTimeSteppers([](auto & ts) { ts.postStep(); });
}

bool
CompositionDT::constrainStep(Real & dt)
{
  bool at_sync_point = TimeStepper::constrainStep(dt);
  const auto time_steppers = getTimeSteppers();
  for (auto & ts : time_steppers)
    if (ts->constrainStep(dt))
      return true;
  return at_sync_point;
}

Real
CompositionDT::computeInitialDT()
{
  return _has_initial_dt ? _initial_dt : computeDT();
}

Real
CompositionDT::computeDT()
{
  const auto time_steppers = getTimeSteppers();
  // Note : compositionDT requires other active time steppers as input so no active time steppers
  // or only compositionDT is active is not allowed
  if (time_steppers.size() < 1)
    mooseError("No TimeStepper(s) are currently active to compute a timestep");

  std::set<std::pair<Real, TimeStepper *>, CompareFirst> dts, bound_dt;

  for (auto & ts : time_steppers)
    if (!dynamic_cast<TimeSequenceStepperBase *>(ts))
    {
      ts->computeStep();
      const auto dt = ts->getCurrentDT();

      if (_lower_bound.count(ts->name()))
        bound_dt.emplace(dt, ts);
      else
        dts.emplace(dt, ts);
    }

  _current_time_stepper = dts.size() ? dts.begin()->second : nullptr;
  _largest_bound_time_stepper = bound_dt.size() ? (--bound_dt.end())->second : nullptr;

  _dt = produceCompositionDT(dts, bound_dt);

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
      _closest_time_sequence_stepper = tss;
      next_time_to_hit = ts_time_to_hit;
    }
  }
  return next_time_to_hit;
}

Real
CompositionDT::produceCompositionDT(
    std::set<std::pair<Real, TimeStepper *>, CompareFirst> & dts,
    std::set<std::pair<Real, TimeStepper *>, CompareFirst> & bound_dts)
{
  Real minDT, lower_bound, dt;
  minDT = lower_bound = dt = 0.0;
  if (!dts.empty())
    minDT = dts.begin()->first;
  if (!bound_dts.empty())
    lower_bound = bound_dts.rbegin()->first;

  if (minDT > lower_bound)
    dt = minDT;
  else
  {
    dt = lower_bound;
    _current_time_stepper = _largest_bound_time_stepper;
  }

  auto ts = getSequenceSteppersNextTime();

  if (ts != 0 && (ts - _time) < dt)
  {
    _current_time_stepper = _closest_time_sequence_stepper;
    return std::min((ts - _time), dt);
  }
  else
    return dt;
}

std::vector<TimeStepper *>
CompositionDT::getTimeSteppers()
{
  std::vector<TimeStepper *> time_steppers;
  _fe_problem.theWarehouse()
      .query()
      .condition<AttribSystem>("TimeStepper")
      .queryInto(time_steppers);

  // Remove CompositionDT from time_steppers vector to avoid recursive call
  time_steppers.erase(std::remove(time_steppers.begin(), time_steppers.end(), this),
                      time_steppers.end());
  return time_steppers;
}

void
CompositionDT::step()
{
  if (_current_time_stepper)
    _current_time_stepper->step();
  else
    TimeStepper::step();
}

void
CompositionDT::acceptStep()
{
  actOnTimeSteppers([](auto & ts) { ts.acceptStep(); });
}

void
CompositionDT::rejectStep()
{
  actOnTimeSteppers([](auto & ts) { ts.rejectStep(); });
}

bool
CompositionDT::converged() const
{
  return _current_time_stepper ? _current_time_stepper->converged() : TimeStepper::converged();
}
