//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FEProblem.h"

/**
 * FEProblemBase derived class for testing out PerfGraph
 */
class SlowProblem : public FEProblem
{
public:
  static InputParameters validParams();

  SlowProblem(const InputParameters & params);
  virtual void solve(unsigned int) override;

protected:
  /// Another timed routine
  void otherTimedSection() const;
  /// Get the time to sleep for
  Real getDelay() const;

  const std::vector<Real> _seconds_to_sleep;
  const bool _nested_print;
  const bool _nested_section;
};
