//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "InputParameters.h"
#include "MultiMooseEnum.h"
#include "gtest/gtest.h"

TEST(InputParameters, checkControlParamPrivateError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.addPrivateParam<Real>("private", 1);
    params.declareControllable("private");
    params.checkParams("");
    FAIL() << "checkParams failed to catch private control param";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("private parameter '' marked controllable") != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

// This tests for the bug https://github.com/idaholab/moose/issues/8586.
// It makes sure that range-checked input file parameters comparison functions
// do absolute floating point comparisons instead of using a default epsilon.
TEST(InputParameters, checkRangeCheckedParam)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.addRangeCheckedParam<Real>("p", 1.000000000000001, "p = 1", "Some doc");
    params.checkParams("");
    FAIL() << "range checked input param failed to catch 1.000000000000001 != 1";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Range check failed for param") != std::string::npos)
        << "range check failed with unexpected error: " << msg;
  }
}

TEST(InputParameters, checkControlParamTypeError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.addParam<PostprocessorName>("pp_name", "make_it_valid", "Some doc");
    params.declareControllable("pp_name");
    params.checkParams("");
    FAIL() << "checkParams failed to catch invalid control param type";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("non-controllable type") != std::string::npos)
        << "failed with unexpected error:" << msg;
  }
}

TEST(InputParameters, checkControlParamValidError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.declareControllable("not_valid");
    params.checkParams("");
    FAIL() << "checkParams failed to catch invalid control param";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("invalid parameter '' marked controllable") != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(InputParameters, checkSuppressedError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.suppressParameter<int>("nonexistent");
    FAIL() << "failed to error on supression of nonexisting parameter";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Unable to suppress nonexistent parameter") != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(InputParameters, checkSetDocString)
{
  InputParameters params = emptyInputParameters();
  params.addParam<Real>("little_guy", "What about that little guy?");
  params.setDocString("little_guy", "That little guy, I wouldn't worry about that little_guy.");
  ASSERT_TRUE(params.getDocString("little_guy")
                  .find("That little guy, I wouldn't worry about that little_guy.") !=
              std::string::npos)
      << "retrieved doc string has unexpected value '" << params.getDocString("little_guy") << "'";
}

TEST(InputParameters, checkSetDocStringError)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.setDocString("little_guy", "That little guy, I wouldn't worry about that little_guy.");
    FAIL() << "failed to error on attempt to set non-existing parameter";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Unable to set the documentation string (using setDocString)") !=
                std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

void
testBadParamName(const std::string & name)
{
  try
  {
    InputParameters params = emptyInputParameters();
    params.addParam<bool>(name, "Doc");
    FAIL() << "failed to error on attempt to set invalid parameter name";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_TRUE(msg.find("Invalid parameter name") != std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

/// Just make sure we don't allow invalid parameter names
TEST(InputParameters, checkParamName)
{
  testBadParamName("p 0");
  testBadParamName("p-0");
  testBadParamName("p!0");
}

TEST(InputParameters, applyParameter)
{
  InputParameters p1 = emptyInputParameters();
  p1.addParam<MultiMooseEnum>("enum", MultiMooseEnum("foo=0 bar=1", "foo"), "testing");
  EXPECT_TRUE(p1.get<MultiMooseEnum>("enum").contains("foo"));

  InputParameters p2 = emptyInputParameters();
  p2.addParam<MultiMooseEnum>("enum", MultiMooseEnum("foo=42 bar=43", "foo"), "testing");
  EXPECT_TRUE(p2.get<MultiMooseEnum>("enum").contains("foo"));

  p2.set<MultiMooseEnum>("enum") = "bar";
  p1.applyParameter(p2, "enum");
  EXPECT_TRUE(p1.get<MultiMooseEnum>("enum").contains("bar"));
}

TEST(InputParameters, applyParameters)
{
  // First enum
  InputParameters p1 = emptyInputParameters();
  p1.addParam<MultiMooseEnum>("enum", MultiMooseEnum("foo=0 bar=1", "foo"), "testing");
  EXPECT_TRUE(p1.get<MultiMooseEnum>("enum").contains("foo"));
  EXPECT_FALSE(p1.get<MultiMooseEnum>("enum").contains("bar"));

  // Second enum
  InputParameters p2 = emptyInputParameters();
  p2.addParam<MultiMooseEnum>("enum", MultiMooseEnum("foo=42 bar=43", "foo"), "testing");
  EXPECT_TRUE(p2.get<MultiMooseEnum>("enum").contains("foo"));
  EXPECT_FALSE(p2.get<MultiMooseEnum>("enum").contains("bar"));

  // Change second and apply to first
  p2.set<MultiMooseEnum>("enum") = "bar";
  p1.applyParameters(p2);
  EXPECT_TRUE(p1.get<MultiMooseEnum>("enum").contains("bar"));
  EXPECT_FALSE(p1.get<MultiMooseEnum>("enum").contains("foo"));

  // Change back first (in "quiet_mode") then reapply second, first should change again
  p1.set<MultiMooseEnum>("enum", true) = "foo";
  EXPECT_FALSE(p1.get<MultiMooseEnum>("enum").contains("bar"));
  EXPECT_TRUE(p1.get<MultiMooseEnum>("enum").contains("foo"));
  p1.applyParameters(p2);
  EXPECT_TRUE(p1.get<MultiMooseEnum>("enum").contains("bar"));
  EXPECT_FALSE(p1.get<MultiMooseEnum>("enum").contains("foo"));

  // Change back first then reapply second, first should not change
  p1.set<MultiMooseEnum>("enum") = "foo";
  EXPECT_FALSE(p1.get<MultiMooseEnum>("enum").contains("bar"));
  EXPECT_TRUE(p1.get<MultiMooseEnum>("enum").contains("foo"));
  p1.applyParameters(p2);
  EXPECT_FALSE(p1.get<MultiMooseEnum>("enum").contains("bar"));
  EXPECT_TRUE(p1.get<MultiMooseEnum>("enum").contains("foo"));
}
