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
  Moose::Capabilities capabilities;
  capabilities.add("unittest_bool", true, "Boolean test capability");
  capabilities.Capabilities::add("unittest_bool2", false, "Boolean test capability 2");

  EXPECT_TRUE(capabilities.check("unittest_bool").first);
  EXPECT_FALSE(capabilities.check("!unittest_bool").first);
  EXPECT_TRUE(capabilities.check("!unittest_bool2").first);
  EXPECT_FALSE(capabilities.check("unittest_bool2").first);
  EXPECT_FALSE(capabilities.check("unittest_bool2>10").first);
  EXPECT_FALSE(capabilities.check("unittest_bool2<=10").first);
  EXPECT_FALSE(capabilities.check("unittest_bool2<=1.0.0").first);
  EXPECT_FALSE(capabilities.check("unittest_bool2=>1.0.0").first);
  EXPECT_FALSE(capabilities.check("unittest_doesnotexist").first);
  EXPECT_TRUE(capabilities.check("!unittest_doesnotexist").first);
}

TEST(CapabilitiesTest, intTest)
{
  Moose::Capabilities capabilities;
  capabilities.add("unittest_int", 78, "Integer test capability");

  const std::vector<std::string> is_true = {
      "", "=78", "==78", "<=78", ">=78", "<=79", ">=77", "<79", ">77", "!=77", "!=79"};
  const std::vector<std::string> is_false = {"!=78", ">=79", "<=77", "<78", ">78", "==77"};

  for (const auto & c : is_true)
  {
    EXPECT_TRUE(capabilities.check("unittest_int" + c).first);
    EXPECT_FALSE(capabilities.check("!unittest_int" + c).first);
  }
  for (const auto & c : is_false)
  {
    EXPECT_FALSE(capabilities.check("unittest_int" + c).first);
    EXPECT_TRUE(capabilities.check("!unittest_int" + c).first);
  }
}

TEST(CapabilitiesTest, stringTest)
{
  Moose::Capabilities capabilities;
  capabilities.add("unittest_string", "CLanG", "String test capability");

  EXPECT_TRUE(capabilities.check("unittest_string").first);
  EXPECT_FALSE(capabilities.check("!unittest_string").first);

  EXPECT_TRUE(capabilities.check("unittest_string=clang").first);
  EXPECT_TRUE(capabilities.check("unittest_string=CLANG").first);
  EXPECT_FALSE(capabilities.check("unittest_string=gcc").first);
}

TEST(CapabilitiesTest, versionTest)
{
  Moose::Capabilities capabilities;
  capabilities.add("unittest_version", "3.2.1", "Version number test capability");

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
    EXPECT_TRUE(capabilities.check("unittest_version" + c).first);
    EXPECT_FALSE(capabilities.check("!unittest_version" + c).first);
  }
  for (const auto & c : is_false)
  {
    EXPECT_FALSE(capabilities.check("unittest_version" + c).first);
    EXPECT_TRUE(capabilities.check("!unittest_version" + c).first);
  }
}

TEST(CapabilitiesTest, multipleTest)
{
  Moose::Capabilities capabilities;
  capabilities.add("unittest2_bool", true, "Multiple capability test bool");
  capabilities.add("unittest2_int", 78, "Multiple capability test int");
  capabilities.add("unittest2_string", "CLanG", "Multiple capability test string");
  capabilities.add("unittest2_version", "3.2.1", "Multiple capability test version number");

  EXPECT_TRUE(capabilities
                  .check("!unittest_doesnotexist unittest2_version<4.2.2 "
                         "unittest2_int<100 unittest2_int>50 unittest2_string!=Popel")
                  .first);
}
