//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolutionTimeAdaptiveDT.h"

/**
 * The SolutionTimeAdaptiveDT stepper is pretty important as it is used regularly. However, since it
 * is based on wall time, it is very challenging to ensure consistency in CI testing. This test
 * object derives from SolutionTimeAdaptiveDT and overrides the step() method to use a fixed time
 * sequence instead of wall time. This allows us to test the functionality of the stepper without
 * relying on wall time, which can vary between runs and environments.
 */
class SolutionTimeAdaptiveDTTest : public SolutionTimeAdaptiveDT
{
public:
  static InputParameters validParams();

  SolutionTimeAdaptiveDTTest(const InputParameters & parameters);

protected:
  std::chrono::milliseconds::rep stepAndRecordElapsedTime() override;

  /// A fixed time sequence to simulate wall time for testing purposes.
  const std::vector<long> & _fake_wall_time_sequence;
};
