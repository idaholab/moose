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
