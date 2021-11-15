//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Executor.h"

#include "gtest_include.h"

#include <iostream>
#include <vector>

struct PassFailCase
{
  std::string name;
  std::string input;
};

TEST(ExecutorTests, ResultStr)
{
  Executor::Result r1("foo");
  Executor::Result r2("bar");
  Executor::Result r3("baz");

  r1.pass("glaberdorf danked successfully");
  r2.fail("failed to fromp the strabler");

  r1.record("inner1", r2);
  r1.record("inner2", r3);

  EXPECT_EQ(r1.str(), "foo\n    inner1:bar(FAIL): failed to fromp the strabler\n    inner2:baz\n");
  EXPECT_EQ(r1.str(true),
            "foo(pass): glaberdorf danked successfully\n    inner1:bar(FAIL): failed to fromp the "
            "strabler\n    inner2:baz(pass)\n");
}
