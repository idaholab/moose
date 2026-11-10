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
#include "GeneralPostprocessor.h"

/**
 * This postprocessor outputs the amount of failed time steps.
 */
class NumFailedTimeSteps : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  NumFailedTimeSteps(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override {}
  virtual Real getValue() const override;

protected:
  /// Retrieves the current TimeStepper used by the executioner.
  const TimeStepper * _timestepper;

  /// Whether to count only the failed solves of the current time step
  const bool _current_time_step_only;

  /// Number of failures before the current time step
  unsigned int & _failures_before_step;

  /// Time step number at which _failures_before_step was recorded
  int & _t_step_counted;
};
