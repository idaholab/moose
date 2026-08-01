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
    _set_end_time(getParam<bool>("use_last_t_for_end_time"))
{
}

void
TimeSequenceStepperBase::setupSequence(const std::vector<Real> & times)
{
  // In case of half transient, transient's end time needs to be reset to
  // be able to imprint TimeSequenceStepperBase's end time
  if (_app.testCheckpointHalfTransient())
    _executioner.endTime() = _executioner.endTime() * 2.0 - _executioner.getStartTime();

  // When restarting or recovering, we reload _time_sequence as restartable data
  if (_time_sequence.empty() || (!_app.isRestarting() && !_app.isRecovering()))
    updateSequence(times);
  else if (_app.isRecovering())
    mooseAssert(_current_step < _time_sequence.size() &&
                    _time_sequence[_current_step] - _time <= _timestep_tolerance &&
                    (_current_step + 1 == _time_sequence.size() ||
                     _time_sequence[_current_step + 1] - _time > _timestep_tolerance),
                "The recovered current step must identify the last reached sequence time");
  else
  {
    if (!MooseUtils::absoluteFuzzyEqual(_executioner.getStartTime(), _time_sequence[0]))
      mooseError("Timesequencestepper does not allow the start time to be modified.");

    auto current_input_sequence = buildSequence(times);
    // Count the leading sequence entries already reached at the current time. Entries no greater
    // than _time + _timestep_tolerance are complete, so this count is also the index of the first
    // future entry, or sequence.size() if every entry is complete.
    const auto completed_prefix_size = [this](const auto & sequence)
    {
      return std::distance(sequence.begin(),
                           std::find_if(sequence.begin(),
                                        sequence.end(),
                                        [this](const auto sequence_time)
                                        { return sequence_time - _time > _timestep_tolerance; }));
    };

    const auto saved_prefix_size = completed_prefix_size(_time_sequence);
    const auto current_prefix_size = completed_prefix_size(current_input_sequence);
    if (current_prefix_size != saved_prefix_size)
      mooseError("The timesequence provided in the restart file must be identical to "
                 "the one in the old file through the restart time, but it contains ",
                 current_prefix_size,
                 " completed value(s) instead of ",
                 saved_prefix_size,
                 ".");

    for (const auto j : make_range(saved_prefix_size))
      if (!MooseUtils::absoluteFuzzyEqual(current_input_sequence[j], _time_sequence[j]))
        mooseError("The timesequence provided in the restart file must be identical to "
                   "the one in the old file through the restart time, but entry ",
                   j + 1,
                   " is ",
                   current_input_sequence[j],
                   " in the restart input and ",
                   _time_sequence[j],
                   " in the restarted input.");

    _time_sequence = std::move(current_input_sequence);
    synchronizeCurrentStep(_time, _timestep_tolerance);
  }

  // Set end time to last time in sequence if requested
  if (_set_end_time)
  {
    auto & end_time = _executioner.endTime();
    end_time = _time_sequence.back();
  }

  if (_app.testCheckpointHalfTransient())
  {
    unsigned int half = (_time_sequence.size() - 1) / 2;
    _executioner.endTime() = _time_sequence[half];
  }
}

std::vector<Real>
TimeSequenceStepperBase::buildSequence(const std::vector<Real> & times) const
{
  const Real start_time = _executioner.getStartTime();
  const Real end_time = _executioner.endTime();

  // make sure time sequence is in strictly ascending order
  if (!std::is_sorted(times.begin(), times.end(), std::less_equal<Real>()))
    paramError("time_sequence", "Time points must be in strictly ascending order.");

  std::vector<Real> sequence{start_time};
  for (const auto time : times)
    if (time > start_time && time <= end_time)
      sequence.push_back(time);

  // Always append end_time as a sentinel, even when it duplicates the last supplied time.
  if (!_set_end_time)
    sequence.push_back(end_time);

  return sequence;
}

void
TimeSequenceStepperBase::updateSequence(const std::vector<Real> & times)
{
  _time_sequence = buildSequence(times);
  synchronizeCurrentStep(_time, _timestep_tolerance);
}

void
TimeSequenceStepperBase::resetSequence()
{
  _time_sequence.clear();
}

bool
TimeSequenceStepperBase::advanceToFutureTime(Real time, Real tolerance, Real & next_time)
{
  refreshSequence();
  const auto first_future = findFirstFutureTime(time, tolerance);
  if (first_future == _time_sequence.cend())
    return false;

  next_time = *first_future;
  return true;
}

std::vector<Real>::const_iterator
TimeSequenceStepperBase::findFirstFutureTime(Real time, Real tolerance) const
{
  return std::partition_point(_time_sequence.begin(),
                              _time_sequence.end(),
                              [time, tolerance](const auto sequence_time)
                              { return sequence_time - time <= tolerance; });
}

void
TimeSequenceStepperBase::synchronizeCurrentStep(Real time, Real tolerance)
{
  const auto first_future = findFirstFutureTime(time, tolerance);
  _current_step =
      first_future == _time_sequence.cbegin()
          ? 0
          : static_cast<unsigned int>(std::distance(_time_sequence.cbegin(), first_future) - 1);
}

Real
TimeSequenceStepperBase::getNextTimeInSequence()
{
  refreshSequence();
  mooseAssert(_current_step + 1 < _time_sequence.size(),
              "The time sequence must contain a future time");
  return _time_sequence[_current_step + 1];
}

void
TimeSequenceStepperBase::acceptStep()
{
  TimeStepper::acceptStep();
  refreshSequence();
  while (_current_step + 1 < _time_sequence.size() &&
         _time_sequence[_current_step + 1] - _time <= _timestep_tolerance)
    increaseCurrentStep();
}

Real
TimeSequenceStepperBase::computeInitialDT()
{
  return computeDT();
}

Real
TimeSequenceStepperBase::computeDT()
{
  refreshSequence();
  mooseAssert(_current_step + 1 < _time_sequence.size(),
              "The time sequence must contain a future time");
  const auto next_time = _time_sequence[_current_step + 1];

  if (_use_last_dt_after_last_t)
  {
    // last *provided* time value index; actual last index corresponds to end time
    const auto last_t_index = _time_sequence.size() - 2;
    if (_current_step + 1 > last_t_index)
      return _time_sequence[last_t_index] - _time_sequence[last_t_index - 1];
  }

  return next_time - _time;
}
