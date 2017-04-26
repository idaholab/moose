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

#ifndef TESTPOSTPROCESSOR_H
#define TESTPOSTPROCESSOR_H

#include "GeneralPostprocessor.h"

class TestPostprocessor;

template <>
InputParameters validParams<TestPostprocessor>();

/**
 * A postprocessor for testing
 *
 * The type of test that this Postprocessor is used for may be altered using the
 * "test_type" parameter, the following are available:
 *
 * "grow" Tests the initial solution equality (see #1396)
 * "report_old" Test the setting of old values in the Postprocessors (see #5106)
 */
class TestPostprocessor : public GeneralPostprocessor
{
public:
  TestPostprocessor(const InputParameters & parameters);

  ///@{
  /**
   * These methods are intentionally empty
   */
  virtual ~TestPostprocessor(){};
  virtual void initialize(){};
  virtual void execute(){};
  ///@}

  /**
   * Returns the postprocessor depending on the 'test_type' parameter
   * @return The postprocessor value
   */
  virtual Real getValue();

private:
  /// Type of testing action to perform
  MooseEnum _test_type;

  ///@{
  /// Reference to the old/older value
  const PostprocessorValue & _old_val;
  const PostprocessorValue & _older_val;
  ///@}

  /// A test counter
  unsigned int _execute_count = 0;
};

#endif // TESTPOSTPROCESSOR_H
