//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SolutionTimeAdaptiveDTTest.h"

registerMooseObject("MooseTestApp", SolutionTimeAdaptiveDTTest);

InputParameters
SolutionTimeAdaptiveDTTest::validParams()
{
  InputParameters params = SolutionTimeAdaptiveDT::validParams();
  params.addRequiredParam<std::vector<long>>(
      "fake_wall_time_sequence",
      "A fixed time sequence (in milliseconds) to simulate wall time for testing purposes.");
  return params;
}

SolutionTimeAdaptiveDTTest::SolutionTimeAdaptiveDTTest(const InputParameters & parameters)
  : SolutionTimeAdaptiveDT(parameters),
    _fake_wall_time_sequence(getParam<std::vector<long>>("fake_wall_time_sequence"))
{
}

std::chrono::milliseconds::rep
SolutionTimeAdaptiveDTTest::stepAndRecordElapsedTime()
{
  if (std::size_t(_t_step) >= _fake_wall_time_sequence.size())
    mooseError("Attempting to access a fake wall time that is out of bounds. "
               "Check the size of the 'fake_wall_time_sequence' parameter.");

  TimeStepper::step();
  return static_cast<std::chrono::milliseconds::rep>(_fake_wall_time_sequence[_t_step]);
}
