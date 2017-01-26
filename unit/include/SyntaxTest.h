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

#ifndef SYNTAXTEST_H
#define SYNTAXTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

#include "Syntax.h"

class SyntaxTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(SyntaxTest);

  CPPUNIT_TEST(testGeneral);
  CPPUNIT_TEST(testDeprecated);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp();
  void tearDown();

  void testGeneral();
  void errorChecks();
  void testDeprecated();

private:
  Syntax _syntax;
};

#endif // SYNTAXTEST_H
