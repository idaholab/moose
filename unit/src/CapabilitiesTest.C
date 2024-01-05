//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Capabilities.h"

#include "gtest_include.h"

TEST(CapabilitiesTest, boolTest)
{
  Capabilities::add("unittest_bool");

  EXPECT_TRUE(Capabilities::check("unittest_bool").first);
  EXPECT_FALSE(Capabilities::check("!unittest_bool").first);
  EXPECT_FALSE(Capabilities::check("unittest_doesnotexist").first);
  EXPECT_TRUE(Capabilities::check("!unittest_doesnotexist").first);
}

TEST(CapabilitiesTest, intTest)
{
  Capabilities::add("unittest_int", 78);

  const std::vector<std::string> is_true = {
      "", "=78", "==78", "<=78", ">=78", "<=79", ">=77", "<79", ">77", "!=77", "!=79"};
  const std::vector<std::string> is_false = {"!=78", ">=79", "<=77", "<78", ">78", "==77"};

  for (const auto & c : is_true)
  {
    EXPECT_TRUE(Capabilities::check("unittest_int" + c).first);
    EXPECT_FALSE(Capabilities::check("!unittest_int" + c).first);
  }
  for (const auto & c : is_false)
  {
    EXPECT_FALSE(Capabilities::check("unittest_int" + c).first);
    EXPECT_TRUE(Capabilities::check("!unittest_int" + c).first);
  }
}

TEST(CapabilitiesTest, stringTest)
{
  Capabilities::add("unittest_string", "CLanG");

  EXPECT_TRUE(Capabilities::check("unittest_string").first);
  EXPECT_FALSE(Capabilities::check("!unittest_string").first);

  EXPECT_TRUE(Capabilities::check("unittest_string=clang").first);
  EXPECT_TRUE(Capabilities::check("unittest_string=CLANG").first);
  EXPECT_FALSE(Capabilities::check("unittest_string=gcc").first);
}

TEST(CapabilitiesTest, versionTest)
{
  Capabilities::add("unittest_version", "3.2.1");

  const std::vector<std::string> is_true = {"",
                                            ">2.1",
                                            ">3.1",
                                            ">=3.2",
                                            ">3.2.0",
                                            ">=3.2.1",
                                            "<=3.2.1",
                                            "=3.2.1",
                                            "==3.2.1",
                                            "<=3.2.2",
                                            "<3.2.2",
                                            "<3.3.2",
                                            "<4.2.2",
                                            "<3.3",
                                            "<4.2",
                                            "<4"};
  const std::vector<std::string> is_false = {
      "<3", "<2", "<=3", "=1.2.3", "=3", "==4", "==3.2", ">4", ">=4", ">=3.3"};

  for (const auto & c : is_true)
  {
    EXPECT_TRUE(Capabilities::check("unittest_version" + c).first);
    EXPECT_FALSE(Capabilities::check("!unittest_version" + c).first);
  }
  for (const auto & c : is_false)
  {
    EXPECT_FALSE(Capabilities::check("unittest_version" + c).first);
    EXPECT_TRUE(Capabilities::check("!unittest_version" + c).first);
  }
}

TEST(CapabilitiesTest, multipleTest)
{
  Capabilities::add("unittest2_bool");
  Capabilities::add("unittest2_int", 78);
  Capabilities::add("unittest2_string", "CLanG");
  Capabilities::add("unittest2_version", "3.2.1");

  EXPECT_TRUE(Capabilities::check("!unittest_doesnotexist unittest2_version<4.2.2 "
                                  "unittest2_int<100 unittest2_int>50 unittest2_string!=Popel")
                  .first);
}
