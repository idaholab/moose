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

std::pair<std::string, std::vector<hit::ErrorMessage>>
expand(const std::string & input)
{
  std::unique_ptr<hit::Node> root(hit::parse("TEST", input));
  EnumerateEvaler enumerate_ev;
  hit::BraceExpander exw;
  exw.registerEvaler("enumerate", enumerate_ev);
  root->walk(&exw);
  return {root->param<std::string>("blocks"), exw.errors};
}

TEST(EnumerateEvalerTest, expansion)
{
  const auto [value, errors] = expand("blocks = '${enumerate block0 block3}'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "block0 block1 block2 block3");
}

TEST(EnumerateEvalerTest, singleElementRange)
{
  const auto [value, errors] = expand("blocks = '${enumerate block5 block5}'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "block5");
}

TEST(EnumerateEvalerTest, mixedWithLiterals)
{
  const auto [value, errors] = expand("blocks = 'block0 ${enumerate block1 block4} block5'");
  EXPECT_TRUE(errors.empty());
  EXPECT_EQ(value, "block0 block1 block2 block3 block4 block5");
}

TEST(EnumerateEvalerTest, wrongArgCount)
{
  const auto [value, errors] = expand("blocks = '${enumerate block0}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("Expected 2 arguments"), std::string::npos);
}

TEST(EnumerateEvalerTest, noTrailingInteger)
{
  const auto [value, errors] = expand("blocks = '${enumerate block block3}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("does not end in an integer"), std::string::npos);
}

TEST(EnumerateEvalerTest, mismatchedPrefix)
{
  const auto [value, errors] = expand("blocks = '${enumerate block0 anotherblock3}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("do not match"), std::string::npos);
}

TEST(EnumerateEvalerTest, descendingRange)
{
  const auto [value, errors] = expand("blocks = '${enumerate block3 block0}'");
  ASSERT_EQ(errors.size(), 1);
  EXPECT_NE(errors[0].message.find("is smaller than first index"), std::string::npos);
}
