//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "ADFParser.h"

TEST(ADFparserTest, JITCompile)
{
  std::string func = "2 + 4*x + 8*x^2 + 16*y^2 + 2*y^4";

  // ADFParser only works with JIT support right now
#if LIBMESH_HAVE_FPARSER_JIT
  ADFParser fparser;
  fparser.Parse(func, "x,y");
  fparser.Optimize();
  fparser.JITCompile();

  ADReal v[2] = {1.5, 2.5};
  Moose::derivInsert(v[0].derivatives(), 0, 1);
  Moose::derivInsert(v[1].derivatives(), 1, 1);

  // evaluate parsed function
  auto p = fparser.Eval(v);

  // evaluate statically compiled expression
  auto s = 2 + 4 * v[0] + 8 * v[0] * v[0] + 16 * v[1] * v[1] + 2 * v[1] * v[1] * v[1] * v[1];

  EXPECT_EQ(p, s);
  EXPECT_EQ(p.derivatives()[0], s.derivatives()[0]);
  EXPECT_EQ(p.derivatives()[1], s.derivatives()[1]);
#endif
}
