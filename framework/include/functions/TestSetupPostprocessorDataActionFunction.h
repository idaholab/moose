//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef TESTSETUPPOSTPROCESSORDATAACTIONFUNCTION_H
#define TESTSETUPPOSTPROCESSORDATAACTIONFUNCTION_H

// MOOSE includes
#include "Function.h"

// Forward declerations
class TestSetupPostprocessorDataActionFunction;

template <>
InputParameters validParams<TestSetupPostprocessorDataActionFunction>();

/**
 * A class for testing SetupPostprocessorDataAction. Function are created before Postprocessors but
 * hasPostprocessor should work. This tests that it does.
 */
class TestSetupPostprocessorDataActionFunction : public Function
{
public:
  /**
   * Class constructor
   * @param parameters The parameters object holding data for the class to use.
   */
  TestSetupPostprocessorDataActionFunction(const InputParameters & parameters);
};

#endif // TESTSETUPPOSTPROCESSORDATAACTIONFUNCTION_H
