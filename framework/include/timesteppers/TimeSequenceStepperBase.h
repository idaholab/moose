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

  /**
   * Return the first sequence time that has not yet been reached, if any
   *
   * The search does not advance the current sequence position.
   *
   * @return Whether a future sequence time was found and assigned to \p next_time
   */
  bool advanceToFutureTime(Real time, Real tolerance, Real & next_time);

  virtual void init() override {}
  virtual void acceptStep() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

  /**
   * Re-read a time sequence source that can change during the simulation
   *
   * This hook is called before the stored sequence is accessed. The default implementation is a
   * no-op for fixed sequences. Derived classes with dynamic sources should retrieve the current
   * time points and pass them to updateSequence(), which rebuilds the canonical sequence and
   * synchronizes the current step.
   */
  virtual void refreshSequence() {}

  /// Build the canonical time sequence for the current start and end times
  std::vector<Real> buildSequence(const std::vector<Real> & times) const;

  /// Find the first sequence time greater than \p time by more than \p tolerance
  std::vector<Real>::const_iterator findFirstFutureTime(Real time, Real tolerance) const;

  /**
   * Set the current sequence position from \p time
   *
   * `_current_step` is the cursor into `_time_sequence`: it indexes the last sequence time
   * considered reached. Sequence times less than or equal to \p time plus \p tolerance are treated
   * as reached, so `_current_step + 1` identifies the next future time when one exists.
   */
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
