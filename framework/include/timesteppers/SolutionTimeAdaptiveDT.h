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

#include <fstream>

/**
 *
 */
class SolutionTimeAdaptiveDT : public TimeStepper
{
public:
  static InputParameters validParams();

  SolutionTimeAdaptiveDT(const InputParameters & parameters);
  virtual ~SolutionTimeAdaptiveDT();

  virtual void step() override;

  virtual void rejectStep() override;

protected:
  /// Take a step and record the elapsed time
  virtual std::chrono::milliseconds::rep stepAndRecordElapsedTime();

  virtual Real computeInitialDT() override;
  virtual Real computeDT() override;

  /**
   * Multiplier specifying the direction the timestep is currently going.
   * Positive for up.  Negative for down.
   */
  int & _direction;

  /// Percentage to change the timestep by either way.
  const Real _percent_change;

  /// Ratios to control whether to increase or decrease the current timestep
  Real & _older_sol_time_vs_dt;
  Real & _old_sol_time_vs_dt;
  Real & _sol_time_vs_dt;

  /// Boolean to control whether a separate adapt log is written to a file
  bool _adapt_log;

  /// The filehandle to hold the log
  std::ofstream _adaptive_log;
};
