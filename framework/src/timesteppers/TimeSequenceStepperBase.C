//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeSequenceStepperBase.h"
#include "FEProblem.h"
#include "Transient.h"

#include <algorithm>
#include <functional>

InputParameters
TimeSequenceStepperBase::validParams()
{
  InputParameters params = TimeStepper::validParams();
  params.addParam<bool>(
      "use_last_dt_after_last_t",
      false,
      "If true, uses the final time step size for times after the last time in the sequence, "
      "instead of taking a single step directly to the simulation end time");
  params.addParam<bool>(
      "use_last_t_for_end_time", false, "Use last time in sequence as 'end_time' in Executioner.");
  return params;
}

TimeSequenceStepperBase::TimeSequenceStepperBase(const InputParameters & parameters)
  : TimeStepper(parameters),
    _use_last_dt_after_last_t(getParam<bool>("use_last_dt_after_last_t")),
    _current_step(declareRestartableData<unsigned int>("current_step", 0)),
    _time_sequence(declareRestartableData<std::vector<Real>>("time_sequence")),
    _use_last_t_for_end_time(getParam<bool>("use_last_t_for_end_time"))
{
}

void
TimeSequenceStepperBase::setupSequence(const std::vector<Real> & times)
{
  // In case of half transient, transient's end time needs to be reset to
  // be able to imprint TimeSequenceStepperBase's end time
  if (_app.testCheckpointHalfTransient())
    _executioner.endTime() = _executioner.endTime() * 2.0 - _executioner.getStartTime();

  // only set up _time_sequence if the app is _not_ recovering
  // if recovering, _time_sequence is a restartable data and should recover on its own
  // However, if the time sequence is dynamic, then we need to be updating it anyway
  if (!_app.isRecovering() || times != _time_sequence)
  {
    // also we need to do something different when restarting
    if (!_app.isRestarting() || _time_sequence.empty())
      updateSequence(times);
    else
    {
      // in case of restart it should be allowed to modify _time_sequence if it follows the
      // following rule:
      // all times up to _current_step are identical
      // 1. start time cannot be modified
      // 2. the entries in _time_sequence and times must be equal up to entry with index
      // _current_step

      if (!MooseUtils::absoluteFuzzyEqual(_executioner.getStartTime(), _time_sequence[0]))
        mooseError("Timesequencestepper does not allow the start time to be modified.");

      // make sure time sequence is in ascending order
      for (unsigned int j = 0; j < times.size() - 1; ++j)
        if (times[j + 1] <= times[j])
          mooseError("time_sequence must be in ascending order.");

      if (times.size() < _current_step + 1)
        mooseError("The timesequence provided in the restart file must be identical to "
                   "the one in the old file up to entry number ",
                   _current_step + 1,
                   " but there are only ",
                   times.size(),
                   " value(s) provided for the timesequence in the restart input.");

      // save the restarted time_sequence
      std::vector<Real> saved_time_sequence = _time_sequence;

      _time_sequence.clear();

      // step 1: fill in the entries up to _current_step
      for (unsigned int j = 0; j <= _current_step; ++j)
      {
        if (!MooseUtils::absoluteFuzzyEqual(times[j], saved_time_sequence[j]))
          mooseError("The timesequence provided in the restart file must be identical to "
                     "the one in the old file up to entry number ",
                     _current_step + 1,
                     " but entry ",
                     j + 1,
                     " is ",
                     times[j],
                     " in the restart input but ",
                     saved_time_sequence[j],
                     " in the restarted input.");
        _time_sequence.push_back(saved_time_sequence[j]);
      }

      // step 2: fill in the entries up after _current_step
      for (unsigned int j = _current_step + 1; j < times.size(); ++j)
        _time_sequence.push_back(times[j]);
    }
  }

  // Set end time to last time in sequence if requested
  if (_use_last_t_for_end_time)
  {
    auto & end_time = _executioner.endTime();
    end_time = _time_sequence.back();
  }

  // If we update the time sequence, the _current_step could be wrong now
  // This would happen if before a recover, the sequence was modified
  // and after a recover, the sequence was re-initialized
  for (const auto i : index_range(_time_sequence))
    if (_time_sequence[i] > _time)
    {
      _current_step = i - 1;
      break;
    }

  // Or if testing the checkpoint functionality, in which case
  // the end time is best determined from the sequence
  if (_app.testCheckpointHalfTransient())
  {
    unsigned int half = (_time_sequence.size() - 1) / 2;
    // Avoid using the start time (added to the sequence)
    if (_time_sequence[half] > _executioner.getStartTime())
      _executioner.endTime() = _time_sequence[half];
    else
    {
      mooseAssert(half + 1 < _time_sequence.size(), "Accessing sequence out of bounds");
      _executioner.endTime() = _time_sequence[half + 1];
    }
  }
}

void
TimeSequenceStepperBase::updateSequence(const std::vector<Real> & times)
{
  Real start_time = _executioner.getStartTime();

  // make sure time sequence is in strictly ascending order
  if (!std::is_sorted(times.begin(), times.end(), std::less_equal<Real>()))
    paramError("time_sequence", "Time points must be in strictly ascending order.");

  // Updating the sequence means replacing the current sequence
  _time_sequence.clear();

  // Move the times into the sequence
  // Keep all the times + start + end time, and keep things sorted
  _time_sequence.push_back(start_time);

  for (unsigned int j = 0; j < times.size(); ++j)
    _time_sequence.push_back(times[j]);
}

void
TimeSequenceStepperBase::resetSequence()
{
  _time_sequence.clear();
}

void
TimeSequenceStepperBase::acceptStep()
{
  TimeStepper::acceptStep();
  if (MooseUtils::absoluteFuzzyGreaterEqual(_time, getNextTimeInSequence()))
    increaseCurrentStep();

  // If the time sequence is dynamic, we need to re-compute it every step we take
  // NOTE: we could even need to re-compute it when rejecting steps, not implemented for now
  updateSequence();
}

Real
TimeSequenceStepperBase::computeInitialDT()
{
  return computeDT();
}

Real
TimeSequenceStepperBase::computeDT()
{
  if (_use_last_dt_after_last_t)
  {
    // Use the last *provided* dt computed from the sequence of times
    const auto last_t_index = _time_sequence.size() - 1;
    if (_current_step + 1 > last_t_index)
      return _time_sequence[last_t_index] - _time_sequence[last_t_index - 1];
    else
      return _time_sequence[_current_step + 1] - _time_sequence[_current_step];
  }
  else
  {
    // Note that we don't insert the end time into the sequence
    if (_current_step + 1 < _time_sequence.size())
      return _time_sequence[_current_step + 1] - _time;
    else
      return _executioner.endTime() - _time;
  }
}

Real
TimeSequenceStepperBase::computeFailedDT()
{
  if (computeDT() <= _dt_min)
    mooseError("Solve failed and timestep already at or below dtmin, cannot continue!");

  // cut the time step in a half if possible
  Real dt = _cutback_factor_at_failure * computeDT();
  if (dt < _dt_min)
    dt = _dt_min;

  _time_sequence.insert(_time_sequence.begin() + _current_step + 1,
                        _time_sequence[_current_step] + dt);
  return computeDT();
}
