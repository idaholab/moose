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
 * A postprocessor for testing serialized solution vectors
 */
class TestSerializedSolution : public GeneralPostprocessor
{
public:
  static InputParameters validParams();

  TestSerializedSolution(const InputParameters & parameters);

  /**
   * Reset data
   */
  virtual void initialize();

  /**
   * Sum up all entries in the solution vector - verify the same answer on all processors
   */
  virtual void execute();

  /**
   * Return the summed value.
   */
  virtual Real getValue();

protected:
  /// The system to be tested
  SystemBase & _test_sys;

  /// Reference to the serialized solution for the test system
  NumericVector<Number> & _serialized_solution;

  /// Sum of all of the entries in the serialized solution vector
  Real _sum;
};
