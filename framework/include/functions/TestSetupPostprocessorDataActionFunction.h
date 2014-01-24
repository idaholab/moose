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

template<>
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
   * @param name
   * @param parameters
   */
  TestSetupPostprocessorDataActionFunction(const std::string & name, InputParameters parameters);

  /**
   * Class destructor
   */
  virtual ~TestSetupPostprocessorDataActionFunction();
};

#endif //TESTSETUPPOSTPROCESSORDATAACTIONFUNCTION_H
