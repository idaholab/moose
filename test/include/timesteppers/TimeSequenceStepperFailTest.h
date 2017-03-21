/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

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
