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
#ifndef TESTSTEADY_H
#define TESTSTEADY_H

#include "Steady.h"

class TestSteady;

template <>
InputParameters validParams<TestSteady>();

/**
 * Test executioner to show exception handling
 */
class TestSteady : public Steady
{
public:
  TestSteady(const InputParameters & parameters);
  virtual ~TestSteady();

  /**
   * This will call solve() on the NonlinearSystem.
   */
  virtual void execute() override;

  /**
   * Calls a custom execution flag for testing.
   */
  virtual void postSolve() override;

private:
  /// The type of test that this object is to perform
  MooseEnum _test_type;

  /// A value to report (used for addAttributeReporter test)
  Real _some_value_that_needs_to_be_reported;
};

#endif /* TESTSTEADY_H */
