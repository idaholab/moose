//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TIMESEQUENCESTEPPERFAILTEST_H
#define TIMESEQUENCESTEPPERFAILTEST_H

#include "TimeSequenceStepper.h"

class TimeSequenceStepperFailTest;

template <>
InputParameters validParams<TimeSequenceStepperFailTest>();

/**
 * Exactly the same as TimeSequenceStepper, but fails at predetermined
 * timesteps.
 */
class TimeSequenceStepperFailTest : public TimeSequenceStepper
{
public:
  TimeSequenceStepperFailTest(const InputParameters & parameters);

  virtual bool converged() override;

protected:
  /// stores a copy of the original sequence of time points, is not updated due to failures.
  std::vector<Real> _original_time_sequence;
};

#endif
