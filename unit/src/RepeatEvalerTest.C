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
expandRepeat(const std::string & input)
{
  std::unique_ptr<hit::Node> root(hit::parse("TEST", input));
  RepeatEvaler repeat_ev;
  hit::BraceExpander exw;
  exw.registerEvaler("repeat", repeat_ev);
  root->walk(&exw);
  return {root->param<std::string>("blocks"), exw.errors};
}

TEST(RepeatEvalerTest, expansion)
{
  const auto [value, errors] = expandRepeat("blocks = '${repeat foo 3}'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "foo foo foo");
}

TEST(RepeatEvalerTest, wrongArgCount)
{
  const auto [value, errors] = expandRepeat("blocks = '${repeat foo}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("Expected 2 arguments"), std::string::npos);
}

TEST(RepeatEvalerTest, nonIntegerCount)
{
  const auto [value, errors] = expandRepeat("blocks = '${repeat foo x}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("is not a non-negative integer"), std::string::npos);
}
