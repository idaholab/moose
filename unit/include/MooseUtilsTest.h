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

#ifndef MOOSEUTILSTEST_H
#define MOOSEUTILSTEST_H

// CPPUnit includes
#include "GuardedHelperMacros.h"

// Forward declarations
class MooseMesh;
class FEProblemBase;
class Factory;
class MooseApp;

class MooseUtilsTest : public CppUnit::TestFixture
{

  CPPUNIT_TEST_SUITE(MooseUtilsTest);

  CPPUNIT_TEST(camelCaseToUnderscore);
  CPPUNIT_TEST(underscoreToCamelCase);

  CPPUNIT_TEST_SUITE_END();

public:
  void camelCaseToUnderscore();
  void underscoreToCamelCase();
};

#endif // MOOSEUTILSTEST_H
