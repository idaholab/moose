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
  // The half-transient test mechanism runs the simulation to the midpoint of a doubled time
  // range, saves a checkpoint, then recovers and runs the second half. Double end_time here so
  // that the full doubled range is available when building the sequence; the half-point is
  // imposed at the bottom of this function.
  if (_app.testCheckpointHalfTransient())
    _executioner.endTime() = _executioner.endTime() * 2.0 - _executioner.getStartTime();

  if ((!_app.isRestarting() && !_app.isRecovering()) || _time_sequence.empty())
  {
    // Fresh run: build _time_sequence directly from the input times.
    updateSequence(times);
  }
  else if (_app.isRecovering())
  {
    // Recover: the checkpoint has _time_sequence and _current_step, but both may be unreliable:
    //   - computeFailedDT() may have inserted intermediate retry points that are not in the
    //     input, shifting what _current_step points to relative to the new sequence.
    //   - When multiple TimeSequenceSteppers coexist under CompositionDT, each one doubles
    //     and then resets _executioner.endTime() in sequence. The cascade means later steppers
    //     build their sequences against a partially-reduced end time, filtering out time points
    //     that should appear in the full run. Using the saved sequence on recover would
    //     permanently lose those points.
    // Solution: rebuild the sequence fresh from the input (using the correct, un-cascaded
    // end_time), then recompute _current_step by scanning for the last sequence entry that
    // is at or before the recovered simulation time.

    if (!MooseUtils::absoluteFuzzyEqual(_executioner.getStartTime(), _time_sequence[0]))
      mooseError("Timesequencestepper does not allow the start time to be modified.");

    Real end_time = _executioner.endTime();

    // Make sure time sequence is in ascending order.
    for (unsigned int j = 0; j + 1 < times.size(); ++j)
      if (times[j + 1] <= times[j])
        mooseError("time_sequence must be in ascending order.");

    _time_sequence.clear();
    _time_sequence.push_back(_executioner.getStartTime());
    for (const auto time : times)
      if (time > _executioner.getStartTime() && time <= end_time)
        _time_sequence.push_back(time);
    // Push end_time as a sentinel so that computeDT() always has a valid [_current_step+1]
    // to read, even when _current_step sits on the last user-specified time point.
    if (!_set_end_time)
      _time_sequence.push_back(end_time);

    // _current_step must index the last entry that has already been reached. Scan forward
    // until we find an entry strictly after the recovered time.
    // absoluteFuzzyLessThan(a, b) is true when a < b - tol, so its negation covers a >= b - tol,
    // i.e. _time_sequence[j] is within tolerance of or behind _time.
    _current_step = 0;
    for (unsigned int j = 1; j < _time_sequence.size(); ++j)
    {
      if (!MooseUtils::absoluteFuzzyLessThan(_time, _time_sequence[j]))
        _current_step = j;
      else
        break;
    }
  }
  else
  {
    // Restart: the simulation is being continued from a checkpoint written during a deliberate
    // restart (not a crash recovery). The user may supply a modified future sequence, but the
    // completed portion must be identical to the original run so that the restarted solution is
    // consistent with what was computed before the restart point.
    if (!MooseUtils::absoluteFuzzyEqual(_executioner.getStartTime(), _time_sequence[0]))
      mooseError("Timesequencestepper does not allow the start time to be modified.");

    Real end_time = _executioner.endTime();

    // Make sure time sequence is in ascending order.
    for (unsigned int j = 0; j + 1 < times.size(); ++j)
      if (times[j + 1] <= times[j])
        mooseError("time_sequence must be in ascending order.");

    // Build the full sequence that the current input implies for this run.
    std::vector<Real> current_input_sequence;
    current_input_sequence.push_back(_executioner.getStartTime());
    for (const auto time : times)
      if (time > _executioner.getStartTime() && time <= end_time)
        current_input_sequence.push_back(time);
    if (!_set_end_time)
      current_input_sequence.push_back(end_time);

    if (current_input_sequence.size() < _current_step + 1)
      mooseError("The timesequence provided in the restart file must be identical to "
                 "the one in the old file up to entry number ",
                 _current_step + 1,
                 " but there are only ",
                 current_input_sequence.size(),
                 " value(s) provided for the timesequence in the restart input.");

    // Verify that the new input agrees with the completed steps from the checkpoint.
    const std::vector<Real> saved_time_sequence = _time_sequence;

    for (unsigned int j = 0; j <= _current_step; ++j)
    {
      if (!MooseUtils::absoluteFuzzyEqual(current_input_sequence[j], saved_time_sequence[j]))
        mooseError("The timesequence provided in the restart file must be identical to "
                   "the one in the old file up to entry number ",
                   _current_step + 1,
                   " but entry ",
                   j + 1,
                   " is ",
                   current_input_sequence[j],
                   " in the restart input but ",
                   saved_time_sequence[j],
                   " in the restarted input.");
    }

    _time_sequence = std::move(current_input_sequence);
  }

  // Set end time to last time in sequence if requested
  if (_set_end_time)
  {
    auto & end_time = _executioner.endTime();
    end_time = _time_sequence.back();
  }

  // For the half-transient test mechanism, clamp end_time to the sequence midpoint so the
  // first half of the run stops there and writes a checkpoint.
  if (_app.testCheckpointHalfTransient())
  {
    unsigned int half = (_time_sequence.size() - 1) / 2;
    _executioner.endTime() = _time_sequence[half];
  }
}

void
TimeSequenceStepperBase::updateSequence(const std::vector<Real> & times)
{
  Real start_time = _executioner.getStartTime();
  Real end_time = _executioner.endTime();

  // make sure time sequence is in strictly ascending order
  if (!std::is_sorted(times.begin(), times.end(), std::less_equal<Real>()))
    paramError("time_sequence", "Time points must be in strictly ascending order.");

  _time_sequence.push_back(start_time);
  for (unsigned int j = 0; j < times.size(); ++j)
  {
    if (times[j] > start_time && times[j] <= end_time)
      _time_sequence.push_back(times[j]);
  }

  // Always push end_time as a sentinel even if it duplicates the last user-specified time.
  // computeDT() reads _time_sequence[_current_step + 1], so _current_step must never point
  // to the last element; the sentinel guarantees that invariant holds.
  if (!_set_end_time)
    _time_sequence.push_back(end_time);
}

void
TimeSequenceStepperBase::resetSequence()
{
  _time_sequence.clear();
}

bool
TimeSequenceStepperBase::advanceToFutureTime(Real time, Real tolerance)
{
  // Advance _current_step past any sequence entries that have already been reached (within
  // tolerance). Used by CompositionDT to find the nearest upcoming sequence time across all
  // active TimeSequenceSteppers.
  while (_current_step + 1 < _time_sequence.size() && getNextTimeInSequence() - time <= tolerance)
    increaseCurrentStep();

  return getNextTimeInSequence() - time > tolerance;
}

void
TimeSequenceStepperBase::acceptStep()
{
  TimeStepper::acceptStep();
  // Advance _current_step to the last sequence entry that is strictly less than the just-accepted
  // time, so that _time_sequence[_current_step] == _time after a normal step. Using
  // absoluteFuzzyLessThan (a < b - tol) means we stop AT the current time rather than advancing
  // past it, which keeps computeDT() returning the correct next interval.
  // The loop (rather than a single increment) handles recover, where the simulation may resume
  // several sequence points ahead of the initial _current_step.
  while (_current_step + 1 < _time_sequence.size() &&
         MooseUtils::absoluteFuzzyLessThan(_time_sequence[_current_step], _time))
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
  // _current_step always points to the last completed sequence time, so [_current_step + 1]
  // is the next target. The sentinel end_time appended by updateSequence/setupSequence
  // ensures this read is always in bounds.
  if (_use_last_dt_after_last_t)
  {
    // last *provided* time value index; actual last index corresponds to end time
    const auto last_t_index = _time_sequence.size() - 2;
    if (_current_step + 1 > last_t_index)
      return _time_sequence[last_t_index] - _time_sequence[last_t_index - 1];
    else
      return _time_sequence[_current_step + 1] - _time_sequence[_current_step];
  }
  else
    return _time_sequence[_current_step + 1] - _time_sequence[_current_step];
}

Real
TimeSequenceStepperBase::computeFailedDT()
{
  if (computeDT() <= _dt_min)
    mooseError("Solve failed and timestep already at or below dtmin, cannot continue!");

  // Cut the time step by the cutback factor (typically 0.5) and insert a new intermediate
  // target into the sequence immediately after _current_step. The next computeDT() call
  // will then return the shorter interval to the inserted point.
  Real dt = _cutback_factor_at_failure * computeDT();
  if (dt < _dt_min)
    dt = _dt_min;
  _time_sequence.insert(_time_sequence.begin() + _current_step + 1,
                        _time_sequence[_current_step] + dt);
  return computeDT();
}
