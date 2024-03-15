//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Capabilities.h"
#include "libmesh/int_range.h"

#include "gtest/gtest.h"

TEST(CapabilitiesTest, boolTest)
{
  Moose::Capabilities capabilities;
  capabilities.add("unittest_bool", true, "Boolean test capability");
  capabilities.Capabilities::add("unittest_bool2", false, "Boolean test capability 2");

  EXPECT_TRUE(std::get<bool>(capabilities.check("unittest_bool")));
  EXPECT_FALSE(std::get<bool>(capabilities.check("!unittest_bool")));
  EXPECT_TRUE(std::get<bool>(capabilities.check("!unittest_bool2")));
  EXPECT_FALSE(std::get<bool>(capabilities.check("unittest_bool2")));
  EXPECT_FALSE(std::get<bool>(capabilities.check("unittest_bool2>10")));
  EXPECT_FALSE(std::get<bool>(capabilities.check("unittest_bool2<=10")));
  EXPECT_FALSE(std::get<bool>(capabilities.check("unittest_bool2<=1.0.0")));
  EXPECT_FALSE(std::get<bool>(capabilities.check("unittest_bool2=>1.0.0")));
  EXPECT_FALSE(std::get<bool>(capabilities.check("unittest_doesnotexist")));
  EXPECT_TRUE(std::get<bool>(capabilities.check("!unittest_doesnotexist")));
  EXPECT_THROW(capabilities.check("unittest_bool="), std::invalid_argument);
  EXPECT_THROW(capabilities.check("unittest_bool=="), std::invalid_argument);
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
    EXPECT_TRUE(std::get<bool>(capabilities.check("unittest_int" + c)));
    EXPECT_FALSE(std::get<bool>(capabilities.check("!unittest_int" + c)));
  }
  for (const auto & c : is_false)
  {
    EXPECT_FALSE(std::get<bool>(capabilities.check("unittest_int" + c)));
    EXPECT_TRUE(std::get<bool>(capabilities.check("!unittest_int" + c)));
  }
  EXPECT_THROW(capabilities.check("unittest_int<"), std::invalid_argument);
}

TEST(CapabilitiesTest, stringTest)
{
  Moose::Capabilities capabilities;
  capabilities.add("unittest_string", "CLanG", "String test capability");

  EXPECT_TRUE(std::get<bool>(capabilities.check("unittest_string")));
  EXPECT_FALSE(std::get<bool>(capabilities.check("!unittest_string")));

  EXPECT_TRUE(std::get<bool>(capabilities.check("unittest_string=clang")));
  EXPECT_TRUE(std::get<bool>(capabilities.check("unittest_string=CLANG")));
  EXPECT_FALSE(std::get<bool>(capabilities.check("unittest_string=gcc")));
  EXPECT_THROW(capabilities.check("unittest_string>"), std::invalid_argument);
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
    EXPECT_TRUE(std::get<bool>(capabilities.check("unittest_version" + c)));
    EXPECT_FALSE(std::get<bool>(capabilities.check("!unittest_version" + c)));
  }
  for (const auto & c : is_false)
  {
    EXPECT_FALSE(std::get<bool>(capabilities.check("unittest_version" + c)));
    EXPECT_TRUE(std::get<bool>(capabilities.check("!unittest_version" + c)));
  }
  EXPECT_THROW(capabilities.check("!unittest_version<"), std::invalid_argument);
}

TEST(CapabilitiesTest, multipleTest)
{
  using libMesh::index_range;
  Moose::Capabilities capabilities;
  capabilities.add("unittest2_bool", true, "Multiple capability test bool");
  capabilities.add("unittest2_int", 78, "Multiple capability test int");
  capabilities.add("unittest2_string", "CLanG", "Multiple capability test string");
  capabilities.add("unittest2_version", "3.2.1", "Multiple capability test version number");

  EXPECT_TRUE(std::get<bool>(
      capabilities.check("!unittest_doesnotexist unittest2_version<4.2.2 "
                         "unittest2_int<100 unittest2_int>50 unittest2_string!=Popel")));
  EXPECT_THROW(capabilities.check("unittest2_bool unittest2_int< unittest2_string==Popel"),
               std::invalid_argument);

  // test mix of true and false requirements (even indices are true, odd are false)
  const std::vector<std::string> test = {"unittest2_bool",
                                         "!unittest2_bool",
                                         "!unittest_doesnotexist",
                                         "unittest_doesnotexist",
                                         "unittest2_int<100",
                                         "unittest2_int>100",
                                         "unittest2_string=clang",
                                         "unittest2_string=gcc",
                                         "unittest2_version<4.2.2",
                                         "unittest2_version>4.2.2"};
  for (const auto i : index_range(test))
    for (const auto j : index_range(test))
      for (const auto k : index_range(test))
        EXPECT_EQ(std::get<bool>(capabilities.check(test[i] + ' ' + test[j] + ' ' + test[k])),
                  i % 2 + j % 2 + k % 2 == 0);
}

#define BOOST_PARSER_DISABLE_HANA_TUPLE
#include <boost/parser/parser.hpp>

#include <iostream>
#include <string>

namespace bp = boost::parser;

TEST(CapabilitiesTest, spiritParser)
{
  
}