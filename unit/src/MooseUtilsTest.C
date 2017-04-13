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

#include "gtest/gtest.h"

// Moose includes
#include "MooseUtils.h"

TEST(MooseUtils, camelCaseToUnderscore)
{
  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("Foo"), "foo");
  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("FooBar"), "foo_bar");
  EXPECT_EQ(MooseUtils::camelCaseToUnderscore("fooBar"), "foo_bar");
}

TEST(MooseUtils, underscoreToCamelCase)
{
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("foo", false), "foo");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("foo_bar", false), "fooBar");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("_foo_bar", false), "FooBar");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("_foo_bar_", false), "FooBar");

  EXPECT_EQ(MooseUtils::underscoreToCamelCase("foo", true), "Foo");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("foo_bar", true), "FooBar");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("_foo_bar", true), "FooBar");
  EXPECT_EQ(MooseUtils::underscoreToCamelCase("_foo_bar_", true), "FooBar");
}
