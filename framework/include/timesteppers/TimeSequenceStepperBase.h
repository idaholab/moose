//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TimeStepper.h"

/**
 * Solves the PDEs at a sequence of given time points.
 * Adjusts the time sequence vector according to Transient start_time and end_time.
 */
class TimeSequenceStepperBase : public TimeStepper
{
public:
  static InputParameters validParams();

  TimeSequenceStepperBase(const InputParameters & parameters);

  /// Clear the time sequence array, usually use when time sequence need to be updated during the
  /// simulation
  void resetSequence();

  /// Increase the current step count by one
  void increaseCurrentStep() { _current_step++; };

  /// Get the time of the current step from input time sequence
  /// If already at the end, returns a very large number
  virtual Real getNextTimeInSequence()
  {
    if (_current_step < _time_sequence.size())
      return _time_sequence[_current_step];
    else
      // Some consumer could add to this number, avoid overflow
      return std::numeric_limits<Real>::max() / 2;
  };

  virtual void init() override {}
  virtual void acceptStep() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;
  virtual Real computeFailedDT() override;

  /// Callback used to update the sequence in time steppers that might have a dynamic sequence
  virtual void updateSequence() {}
  /// Used for both updating and setting up the sequence: selects different behaviors depending on whether the app is restarting, restoring, initializing, etc
  void setupSequence(const std::vector<Real> & times);
  /// Simply overrites the sequence with the input sequence, with some checks
  void updateSequence(const std::vector<Real> & times);

  /// Whether to use the final dt past the last t in sequence
  const bool _use_last_dt_after_last_t;

  /// the step that the time stepper is currently at
  unsigned int & _current_step;

  /// stores the sequence of time points
  std::vector<Real> & _time_sequence;

  /// Whether to use the last t in sequence as Executioner end_time
  const bool _use_last_t_for_end_time;
};
