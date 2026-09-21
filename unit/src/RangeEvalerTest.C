//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "Parser.h"

static std::pair<std::string, std::vector<hit::ErrorMessage>>
expandRange(const std::string & input)
{
  std::unique_ptr<hit::Node> root(hit::parse("TEST", input));
  RangeEvaler range_ev;
  hit::BraceExpander exw;
  exw.registerEvaler("range", range_ev);
  root->walk(&exw);
  return {root->param<std::string>("blocks"), exw.errors};
}

TEST(RangeEvalerTest, expansion)
{
  const auto [value, errors] = expandRange("blocks = '${range 0 3}'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "0 1 2 3");
}

TEST(RangeEvalerTest, singleElementRange)
{
  const auto [value, errors] = expandRange("blocks = '${range 5 5}'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "5");
}

TEST(RangeEvalerTest, mixedWithLiterals)
{
  const auto [value, errors] = expandRange("blocks = '0 ${range 1 4} 5'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "0 1 2 3 4 5");
}

TEST(RangeEvalerTest, wrongArgCount)
{
  const auto [value, errors] = expandRange("blocks = '${range 0}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("Expected 2 arguments"), std::string::npos);
}

TEST(RangeEvalerTest, nonIntegerIndex)
{
  const auto [value, errors] = expandRange("blocks = '${range a 3}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("is not a non-negative integer"), std::string::npos);
}

TEST(RangeEvalerTest, descendingRange)
{
  const auto [value, errors] = expandRange("blocks = '${range 3 0}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("is smaller than first index"), std::string::npos);
}
