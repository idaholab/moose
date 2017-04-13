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

#include "TimeSequenceStepperFailTest.h"

template <>
InputParameters
validParams<TimeSequenceStepperFailTest>()
{
  InputParameters params = validParams<TimeSequenceStepper>();
  return params;
}

TimeSequenceStepperFailTest::TimeSequenceStepperFailTest(const InputParameters & parameters)
  : TimeSequenceStepper(parameters),
    _original_time_sequence(getParam<std::vector<Real>>("time_sequence"))
{
}

bool
TimeSequenceStepperFailTest::converged()
{
  // The goal is to fail exactly one timestep which matches its
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
        Moose::out << "TimeSequenceStepperFailTest: Simulating failed solve of first timestep.\n");
    return false;
  }

  return TimeStepper::converged();
}
