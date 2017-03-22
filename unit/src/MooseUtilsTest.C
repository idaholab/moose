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

#include "MooseUtilsTest.h"

// Moose includes
#include "MooseUtils.h"

CPPUNIT_TEST_SUITE_REGISTRATION(MooseUtilsTest);

void
MooseUtilsTest::camelCaseToUnderscore()
{
  CPPUNIT_ASSERT(MooseUtils::camelCaseToUnderscore("Foo") == "foo");
  CPPUNIT_ASSERT(MooseUtils::camelCaseToUnderscore("FooBar") == "foo_bar");
  CPPUNIT_ASSERT(MooseUtils::camelCaseToUnderscore("fooBar") == "foo_bar");
}

void
MooseUtilsTest::underscoreToCamelCase()
{
  CPPUNIT_ASSERT(MooseUtils::underscoreToCamelCase("foo", false) == "foo");
  CPPUNIT_ASSERT(MooseUtils::underscoreToCamelCase("foo_bar", false) == "fooBar");
  CPPUNIT_ASSERT(MooseUtils::underscoreToCamelCase("_foo_bar", false) == "FooBar");
  CPPUNIT_ASSERT(MooseUtils::underscoreToCamelCase("_foo_bar_", false) == "FooBar");

  CPPUNIT_ASSERT(MooseUtils::underscoreToCamelCase("foo", true) == "Foo");
  CPPUNIT_ASSERT(MooseUtils::underscoreToCamelCase("foo_bar", true) == "FooBar");
  CPPUNIT_ASSERT(MooseUtils::underscoreToCamelCase("_foo_bar", true) == "FooBar");
  CPPUNIT_ASSERT(MooseUtils::underscoreToCamelCase("_foo_bar_", true) == "FooBar");
}
