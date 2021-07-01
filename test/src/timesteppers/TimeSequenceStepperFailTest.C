//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TimeSequenceStepperFailTest.h"

registerMooseObject("MooseTestApp", TimeSequenceStepperFailTest);

InputParameters
TimeSequenceStepperFailTest::validParams()
{
  InputParameters params = TimeSequenceStepper::validParams();
  return params;
}

TimeSequenceStepperFailTest::TimeSequenceStepperFailTest(const InputParameters & parameters)
  : TimeSequenceStepper(parameters),
    _original_time_sequence(getParam<std::vector<Real>>("time_sequence"))
{
}

void
TimeSequenceStepperFailTest::step()
{
  TimeStepper::step();

  // The goal is to fail exactly on the timestep which matches its
  // original sequence point order, other than the initial condition.
  // This can only happen once, since after you fail, you are no
  // longer on the original time sequence (off by one or more).  Since
  // this can only happen once, we only check the [1]th sequence
  // point.  (Having fewer than two sequence points doesn't really
  // make sense...)
  mooseAssert(_original_time_sequence.size() > 1, "Must have at least two sequence points!");

  if (_t_step == 1 && MooseUtils::absoluteFuzzyEqual(_time, _original_time_sequence[1]))
  {
    mooseDoOnce(
        Moose::out << "TimeSequenceStepperFailTest: Simulating failed solve of first timestep."
                   << std::endl);
    _converged = false;
  }

  if (converged())
    _current_step++;
}
