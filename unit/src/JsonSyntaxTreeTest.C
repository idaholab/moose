//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "JsonSyntaxTree.h"

TEST(JsonSyntaxTreeTest, documentationCppTypeUsesReal)
{
  EXPECT_EQ(JsonSyntaxTree::documentationCppType("double"), "Real");
  EXPECT_EQ(JsonSyntaxTree::documentationCppType("std::vector<double>"), "std::vector<Real>");
  EXPECT_EQ(JsonSyntaxTree::documentationCppType("std::vector<std::vector<double>>"),
            "std::vector<std::vector<Real>>");
  EXPECT_EQ(JsonSyntaxTree::documentationCppType("std::map<std::string, double>"),
            "std::map<std::string, Real>");
}

TEST(JsonSyntaxTreeTest, basicCppTypeRecognizesReal)
{
  EXPECT_EQ(JsonSyntaxTree::basicCppType("Real"), "Real");
  EXPECT_EQ(JsonSyntaxTree::basicCppType("std::vector<Real>"), "Array:Real");
}
