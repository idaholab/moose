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

  void setupSequence(const std::vector<Real> & times);
  void updateSequence(const std::vector<Real> & times);

  // Clear the time sequence array, usually used when the time sequence needs to be updated during
  // the simulation
  void resetSequence();

  // Increase the current step count by one
  void increaseCurrentStep() { _current_step++; };

  /// Get the next time in the input time sequence
  virtual Real getNextTimeInSequence();

  /// Advance past sequence times that have already been reached and return the next time, if any
  bool advanceToFutureTime(Real time, Real tolerance, Real & next_time);

  virtual void init() override {}
  virtual void acceptStep() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

  /// Refresh a dynamically changing time sequence before it is used
  virtual void refreshSequence() {}

  /// Build the canonical time sequence for the current start and end times
  std::vector<Real> buildSequence(const std::vector<Real> & times) const;

  /// Set the cursor to the last sequence time reached within tolerance
  void synchronizeCurrentStep(Real time, Real tolerance);

  /// Whether to use the final dt past the last t in sequence
  const bool _use_last_dt_after_last_t;

  /// the step that the time stepper is currently at
  unsigned int & _current_step;

  /// stores the sequence of time points
  std::vector<Real> & _time_sequence;

  /// Whether to use the last t in sequence as Executioner end_time
  const bool _set_end_time;
};
