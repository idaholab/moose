//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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

  // Increase the current step count by one
  void increaseCurrentStep() { _current_step++; };

  // Get the time of the current step from input time sequence
  Real getNextTimeInSequence() { return _time_sequence[_current_step]; };

  virtual void init() override {}
  virtual void step() override;

protected:
  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;
  virtual Real computeFailedDT() override;

  /// the step that the time stepper is currently at
  unsigned int & _current_step;

  /// stores the sequence of time points
  std::vector<Real> & _time_sequence;
};
