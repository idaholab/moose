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

#ifndef EXPRESSIONBUILDERTEST_H
#define EXPRESSIONBUILDERTEST_H

#include "ExpressionBuilder.h"

// CPPUnit includes
#include "GuardedHelperMacros.h"

class ExpressionBuilderTest : public CppUnit::TestFixture, public ExpressionBuilder
{
  /**
   * This macro generates a bunch of boilerplate C++ code
   * for creating a new test suite.
   */
  CPPUNIT_TEST_SUITE(ExpressionBuilderTest);

  /**
   * Adds the passed method name to the suite of tests.
   * You must have a public (?) void member function with
   * that name, taking no arguments, in the class.
   */
  CPPUNIT_TEST(test);

  /**
   * This macro is required at the end of the list of CppUnit
   * macros.  It sets class access back to "private" as well.
   */
  CPPUNIT_TEST_SUITE_END();

public:
  /**
   * Unit test functions must return void and take no arguments.
   */
  void test();
};

#endif // EXPRESSIONBUILDERTEST_H
