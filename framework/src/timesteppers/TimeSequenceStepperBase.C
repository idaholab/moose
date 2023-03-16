//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  return params;
}

TimeSequenceStepperBase::TimeSequenceStepperBase(const InputParameters & parameters)
  : TimeStepper(parameters),
    _current_step(declareRestartableData<unsigned int>("current_step", 0)),
    _time_sequence(declareRestartableData<std::vector<Real>>("time_sequence"))
{
}

void
TimeSequenceStepperBase::setupSequence(const std::vector<Real> & times)
{
  // In case of half transient, transient's end time needs to be reset to
  // be able to imprint TimeSequenceStepperBase's end time
  if (_app.halfTransient())
    _executioner.endTime() *= 2.0;

  // only set up _time_sequence if the app is _not_ recovering
  if (!_app.isRecovering())
  {
    // also we need to do something different when restarting
    if (!_app.isRestarting())
    {
      // sync _executioner.startTime and endTime with _time_sequence
      Real start_time = _executioner.getStartTime();
      Real end_time = _executioner.endTime();

      // make sure time sequence is in strictly ascending order
      if (!std::is_sorted(times.begin(), times.end(), std::less_equal<Real>()))
        paramError("time_sequence", "Time points must be in strictly ascending order.");

      _time_sequence.push_back(start_time);
      for (unsigned int j = 0; j < times.size(); ++j)
      {
        if (times[j] > start_time && times[j] < end_time)
          _time_sequence.push_back(times[j]);
      }
      _time_sequence.push_back(end_time);
    }
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

      // sync _executioner.endTime with _time_sequence
      Real end_time = _executioner.endTime();

      // make sure time sequence is in ascending order
      for (unsigned int j = 0; j < times.size() - 1; ++j)
        if (times[j + 1] <= times[j])
          mooseError("time_sequence must be in ascending order.");

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
                     " = ",
                     saved_time_sequence[_current_step]);

        _time_sequence.push_back(saved_time_sequence[j]);
      }

      // step 2: fill in the entries up after _current_step
      for (unsigned int j = _current_step + 1; j < times.size(); ++j)
      {
        if (times[j] < end_time)
          _time_sequence.push_back(times[j]);
      }
      _time_sequence.push_back(end_time);
    }
  }

  if (_app.halfTransient())
  {
    unsigned int half = (_time_sequence.size() - 1) / 2;
    _executioner.endTime() = _time_sequence[half];
  }
}

void
TimeSequenceStepperBase::step()
{
  TimeStepper::step();
  if (converged() && !_executioner.fixedPointSolve().XFEMRepeatStep())
    _current_step++;
}

Real
TimeSequenceStepperBase::computeInitialDT()
{
  return computeDT();
}

Real
TimeSequenceStepperBase::computeDT()
{
  return _time_sequence[_current_step + 1] - _time_sequence[_current_step];
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
