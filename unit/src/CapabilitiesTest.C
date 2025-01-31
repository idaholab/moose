//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Capabilities.h"
#include "MooseException.h"
#include "libmesh/int_range.h"

#include "gtest/gtest.h"

TEST(CapabilitiesTest, boolTest)
{
  Moose::Capabilities capabilities;
  std::vector<std::pair<std::string, CapabilityUtils::CheckState>> tests = {
      {"unittest_bool", CapabilityUtils::CERTAIN_PASS},
      {"!unittest_bool", CapabilityUtils::CERTAIN_FAIL},
      {"!unittest_bool2", CapabilityUtils::CERTAIN_PASS},
      {"unittest_bool2", CapabilityUtils::CERTAIN_FAIL},
      {"unittest_doesnotexist", CapabilityUtils::POSSIBLE_FAIL},
      {"!unittest_doesnotexist", CapabilityUtils::POSSIBLE_PASS},
  };
  capabilities.add("unittest_bool", true, "Boolean test capability");
  capabilities.Capabilities::add("unittest_bool2", false, "Boolean test capability 2");

  for (const auto & [requirement, state] : tests)
    EXPECT_EQ(std::get<0>(capabilities.check(requirement)), state);

  EXPECT_THROW(capabilities.check("unittest_bool2=>1.0.0"), std::runtime_error);
  EXPECT_THROW(capabilities.check("unittest_bool="), std::runtime_error);
  EXPECT_THROW(capabilities.check("unittest_bool=="), std::runtime_error);
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
    EXPECT_EQ(std::get<0>(capabilities.check("unittest_int" + c)), CapabilityUtils::CERTAIN_PASS);
    EXPECT_EQ(std::get<0>(capabilities.check("!(unittest_int" + c + ")")),
              CapabilityUtils::CERTAIN_FAIL);
  }
  for (const auto & c : is_false)
  {
    EXPECT_EQ(std::get<0>(capabilities.check("unittest_int" + c)), CapabilityUtils::CERTAIN_FAIL);
    EXPECT_EQ(std::get<0>(capabilities.check("!(unittest_int" + c + ")")),
              CapabilityUtils::CERTAIN_PASS);
  }
  EXPECT_THROW(capabilities.check("unittest_int<"), std::runtime_error);
}

TEST(CapabilitiesTest, stringTest)
{
  Moose::Capabilities capabilities;
  capabilities.add("unittest_string", "CLanG", "String test capability");

  EXPECT_EQ(std::get<0>(capabilities.check("unittest_string")), CapabilityUtils::CERTAIN_PASS);
  EXPECT_EQ(std::get<0>(capabilities.check("!unittest_string")), CapabilityUtils::CERTAIN_FAIL);

  EXPECT_EQ(std::get<0>(capabilities.check("unittest_string=clang")),
            CapabilityUtils::CERTAIN_PASS);
  EXPECT_EQ(std::get<0>(capabilities.check("unittest_string=CLANG")),
            CapabilityUtils::CERTAIN_PASS);
  EXPECT_EQ(std::get<0>(capabilities.check("unittest_string=gcc")), CapabilityUtils::CERTAIN_FAIL);
  EXPECT_THROW(capabilities.check("unittest_string>"), std::runtime_error);
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
    EXPECT_EQ(std::get<0>(capabilities.check("unittest_version" + c)),
              CapabilityUtils::CERTAIN_PASS);
    EXPECT_EQ(std::get<0>(capabilities.check("!(unittest_version" + c + ")")),
              CapabilityUtils::CERTAIN_FAIL);
  }
  for (const auto & c : is_false)
  {
    EXPECT_EQ(std::get<0>(capabilities.check("unittest_version" + c)),
              CapabilityUtils::CERTAIN_FAIL);
    EXPECT_EQ(std::get<0>(capabilities.check("!(unittest_version" + c + ")")),
              CapabilityUtils::CERTAIN_PASS);
  }
  EXPECT_THROW(capabilities.check("!unittest_version<"), std::runtime_error);
}

TEST(CapabilitiesTest, multipleTest)
{
  using libMesh::index_range;
  Moose::Capabilities capabilities;
  capabilities.add("unittest_bool", true, "Multiple capability test bool");
  capabilities.add("unittest_int", 78, "Multiple capability test int");
  capabilities.add("unittest_string", "CLanG", "Multiple capability test string");
  capabilities.add("unittest_version", "3.2.1", "Multiple capability test version number");

  std::vector<std::pair<std::string, CapabilityUtils::CheckState>> tests = {
      {"!unittest_doesnotexist & unittest_version<4.2.2 &"
       "unittest_int<100 & unittest_int>50 & unittest_string!=Popel ",
       CapabilityUtils::POSSIBLE_PASS},
      {"!unittest_doesnotexist & unittest_version<4.2.2 &"
       "unittest_int<100 & unittest2_int>50 & unittest_string!=Popel ",
       CapabilityUtils::UNKNOWN},
      {"unittest_doesnotexist & unittest_doesnotexist>2.0.1", CapabilityUtils::POSSIBLE_FAIL},
      {"!unittest_doesnotexist | unittest_doesnotexist<=2.0.1", CapabilityUtils::POSSIBLE_PASS},
      {"unittest_bool & unittest_int!=78", CapabilityUtils::CERTAIN_FAIL},
      {"unittest_bool | unittest_int!=78", CapabilityUtils::CERTAIN_PASS},
      {" !unittest_bool | (unittest_string=gcc | unittest_version<1.0)",
       CapabilityUtils::CERTAIN_FAIL},
      {"(unittest_bool & unittest_int=78) & (unittest_string=clang & unittest_version>2.0)",
       CapabilityUtils::CERTAIN_PASS}};

  for (const auto & [requirement, state] : tests)
    EXPECT_EQ(std::get<0>(capabilities.check(requirement)), state);

  EXPECT_THROW(capabilities.check("unittest2_bool unittest2_int< unittest2_string==Popel"),
               std::runtime_error);
  EXPECT_THROW(capabilities.check("(unittest2_bool"), std::runtime_error);
}

TEST(CapabilitiesTest, parseFail)
{
  Moose::Capabilities capabilities;
  const std::string bad_capabilities = "foo bar";

  const auto result = CapabilityUtils::check(bad_capabilities, capabilities._capability_registry);
  EXPECT_EQ(std::get<0>(result), CapabilityUtils::PARSE_FAIL);

  try
  {
    capabilities.check(bad_capabilities);
    FAIL();
  }
  catch (const std::exception & err)
  {
    ASSERT_EQ(std::string(err.what()),
              "Unable to parse requested capabilities '" + bad_capabilities + "'.");
  }
}
