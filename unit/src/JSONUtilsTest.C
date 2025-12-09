//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "JsonOutputUtils.h"
#include "MooseUnitUtils.h"
#include "gtest/gtest.h"

TEST(JsonOutputUtilsTest, anyToJson)
{
  {
    // int
    int x = 42;
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
  {
    // unsigned int
    unsigned int x = 42;
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
  {
    // string
    std::string x = "test_string";
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
  {
    // Real
    Real x = 3.14;
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
  {
    // bool
    bool x = true;
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
  {
    // vector of ints
    std::vector<int> x = {-1, 2, 3};
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
  {
    // vector of unsigned ints
    std::vector<unsigned int> x = {1, 2, 3};
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
  {
    // vector of strings
    std::vector<std::string> x = {"one", "two", "three"};
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
  {
    // vector of Reals
    std::vector<Real> x = {1.1, 2.2, 3.3};
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
  {
    // vector of bools
    std::vector<bool> x = {true, false, true};
    const std::any data = x;
    const nlohmann::json jout = JsonOutputUtils::anyToJson(data);
    EXPECT_EQ(jout, x);
  }
}