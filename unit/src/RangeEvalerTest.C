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
  EXPECT_EQ(value, "0 1 2");
}

TEST(RangeEvalerTest, expansionProvideStep)
{
  const auto [value, errors] = expandRange("blocks = '${range 0 6 2}'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "0 2 4");
}

TEST(RangeEvalerTest, expansionProvideNegStep)
{
  const auto [value, errors] = expandRange("blocks = '${range 6 0 -2}'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "6 4 2");
}

TEST(RangeEvalerTest, expansionNegVals)
{
  const auto [value, errors] = expandRange("blocks = '${range -4 -1}'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "-4 -3 -2");
}

TEST(RangeEvalerTest, mixedWithLiterals)
{
  const auto [value, errors] = expandRange("blocks = '0 ${range 1 5} 5'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "0 1 2 3 4 5");
}

TEST(RangeEvalerTest, wrongArgCountLess)
{
  const auto [value, errors] = expandRange("blocks = '${range 0}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("Expected either 2 arguments"), std::string::npos);
}

TEST(RangeEvalerTest, wrongArgCountGreater)
{
  const auto [value, errors] = expandRange("blocks = '${range 0 1 2 3}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("or 3 arguments"), std::string::npos);
}

TEST(RangeEvalerTest, nonInteger)
{
  const auto [value, errors] = expandRange("blocks = '${range s 5.5 -}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("is not an integer in"), std::string::npos);
}

TEST(RangeEvalerTest, posStepSmallerEnd)
{
  const auto [value, errors] = expandRange("blocks = '${range 5 0 1}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("is smaller than the start point"), std::string::npos);
}

TEST(RangeEvalerTest, negStepLargerEnd)
{
  const auto [value, errors] = expandRange("blocks = '${range 0 5 -1}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("is smaller than the end point"), std::string::npos);
}

TEST(RangeEvalerTest, zeroStep)
{
  const auto [value, errors] = expandRange("blocks = '${range 0 5 0}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("step is zero in"), std::string::npos);
}
