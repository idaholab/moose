//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

  // The goal is to fail exactly on the first timestep that matches an original sequence point.
  // The retry time differs from that point, and the timestep index has advanced by the time the
  // original point is reached, so this can only happen once.
  mooseAssert(_original_time_sequence.size() > 1, "Must have at least two sequence points!");

  if (_t_step == 1 && MooseUtils::absoluteFuzzyEqual(_time, _original_time_sequence[1]))
  {
    mooseDoOnce(
        Moose::out << "TimeSequenceStepperFailTest: Simulating failed solve of first timestep."
                   << std::endl);
    _converged = false;
  }
}
