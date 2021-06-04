//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Steady.h"

/**
 * Test executioner to show exception handling
 */
class TestSteady : public Steady
{
public:
  static InputParameters validParams();

  TestSteady(const InputParameters & parameters);
  virtual ~TestSteady();

  /**
   * This is called at the beginning of FEProblemBase::init.
   */
  virtual void preProblemInit() override;

  /**
   * This will call solve() on the NonlinearSystem.
   */
  virtual void preExecute() override;

  /**
   * Calls a custom execution flag for testing.
   */
  virtual void postSolve() override;

private:
  /// The type of test that this object is to perform
  MooseEnum _test_type;

  /// A value to report (used for addAttributeReporter test)
  PostprocessorValue * _some_value_that_needs_to_be_reported;
};
