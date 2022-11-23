//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralPostprocessor.h"

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
  static InputParameters validParams();

  TestPostprocessor(const InputParameters & parameters);

  ///@{
  /**
   * These methods are intentionally empty
   */
  virtual ~TestPostprocessor(){};
  virtual void initialize() override {}
  virtual void execute() override {}
  ///@}

  virtual void customSetup(const ExecFlagType & exec_type) override;

  /**
   * Returns the postprocessor depending on the 'test_type' parameter
   * @return The postprocessor value
   */
  virtual Real getValue() override;

private:
  /// Type of testing action to perform
  MooseEnum _test_type;

  ///@{
  /// Reference to the old/older value
  const PostprocessorValue & _old_val;
  const PostprocessorValue & _older_val;
  const PostprocessorValue & _report_old;
  ///@}

  /// A test counter
  unsigned int & _execute_count;
};
