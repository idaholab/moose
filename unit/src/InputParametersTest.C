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
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "Conversion.h"
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
    FAIL() << "checkParams failed to catch non existing control param";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("cannot be marked as controllable") != std::string::npos)
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

TEST(InputParameters, checkSuppressingControllableParam)
{
  InputParameters params = emptyInputParameters();
  params.addParam<bool>("b", "Controllable boolean");
  params.declareControllable("b");
  // Instead of building another identical copy of `param`, we directly suppress the parameter. In a
  // normal situation, suppressing would happen in a child class, but it makes no difference here
  params.suppressParameter<bool>("b");
  // After calling suppressParameter, the controllable parameter should no longer be controllable
  ASSERT_TRUE(params.isControllable("b") == false);
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

TEST(InputParameters, applyParametersVector)
{
  MooseEnum types("foo bar");

  InputParameters p1 = emptyInputParameters();
  p1.addRequiredParam<std::vector<MooseEnum>>("enum", std::vector<MooseEnum>(1, types), "testing");

  InputParameters p2 = emptyInputParameters();
  p2.addRequiredParam<std::vector<MooseEnum>>("enum", std::vector<MooseEnum>(1, types), "testing");

  MooseEnum set("foo bar", "bar");
  p2.set<std::vector<MooseEnum>>("enum") = std::vector<MooseEnum>(1, set);

  EXPECT_FALSE(p1.isParamValid("enum"));
  EXPECT_TRUE(p2.isParamValid("enum"));

  p1.applyParameters(p2);

  EXPECT_TRUE(p1.get<std::vector<MooseEnum>>("enum")[0] == "bar");
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

TEST(InputParameters, applyParametersPrivateOverride)
{
  InputParameters p1 = emptyInputParameters();
  p1.addPrivateParam<bool>("_flag", true);
  EXPECT_TRUE(p1.get<bool>("_flag"));

  InputParameters p2 = emptyInputParameters();
  p2.addPrivateParam<bool>("_flag", false);
  EXPECT_FALSE(p2.get<bool>("_flag"));

  p1.applySpecificParameters(p2, {"_flag"}, true);
  EXPECT_FALSE(p1.get<bool>("_flag"));
}

TEST(InputParameters, makeParamRequired)
{
  InputParameters params = emptyInputParameters();
  params.addParam<Real>("good_param", "documentation");
  params.addParam<Real>("wrong_param_type", "documentation");

  // Require existing parameter with the appropriate type
  EXPECT_FALSE(params.isParamRequired("good_param"));
  params.makeParamRequired<Real>("good_param");
  EXPECT_TRUE(params.isParamRequired("good_param"));

  // Require existing parameter with the wrong type
  try
  {
    params.makeParamRequired<PostprocessorName>("wrong_param_type");
    FAIL() << "failed to error on attempt to change a parameter type when making it required";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_TRUE(msg.find("Unable to require nonexistent parameter: wrong_param_type") !=
                std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  // Require non-existing parameter
  try
  {
    params.makeParamRequired<PostprocessorName>("wrong_param_name");
    FAIL() << "failed to error on attempt to require a non-existent parameter";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    EXPECT_TRUE(msg.find("Unable to require nonexistent parameter: wrong_param_name") !=
                std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(InputParameters, setPPandVofPP)
{
  // programmatically set default value of PPName parameter
  InputParameters p1 = emptyInputParameters();
  p1.addParam<std::vector<PostprocessorName>>("pp_name", "testing");
  p1.set<std::vector<PostprocessorName>>("pp_name") = {"first", "second", "third"};

  // check if we have a vector of pps
  EXPECT_TRUE(p1.isType<std::vector<PostprocessorName>>("pp_name"))
      << "Failed to detect vector of PPs";

  // check what happens if default value is requested
  /*
  try
  {
    p1.hasDefaultPostprocessorValue("pp_name", 2);
    FAIL() << "failed to error on supression of nonexisting parameter";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("Attempting to access _have_default_postprocessor_val") !=
  std::string::npos)
        << "failed with unexpected error: " << msg;
  }
  */

  // EXPECT_EQ(p1.getDefaultPostprocessorValue("pp_name"), 1);
}

TEST(InputParameters, getPairs)
{
  std::vector<std::string> num_words{"zero", "one", "two", "three"};

  InputParameters p = emptyInputParameters();
  p.addParam<std::vector<std::string>>("first", num_words, "");
  p.addParam<std::vector<int>>("second", std::vector<int>{0, 1, 2, 3}, "");

  auto pairs = p.get<std::string, int>("first", "second");

  for (int i = 0; i < 4; ++i)
  {
    EXPECT_EQ(pairs[i].first, num_words[i]);
    EXPECT_EQ(pairs[i].second, i);
  }
}

TEST(InputParameters, getMultiMooseEnumPairs)
{
  std::vector<std::string> v1{"zero", "one", "two", "three"};
  auto s1 = Moose::stringify(v1, " ");
  std::vector<std::string> v2{"null", "eins", "zwei", "drei"};
  auto s2 = Moose::stringify(v2, " ");

  InputParameters p = emptyInputParameters();
  p.addParam<MultiMooseEnum>("first", MultiMooseEnum(s1, s1), "");
  p.addParam<MultiMooseEnum>("second", MultiMooseEnum(s2, s2), "");

  auto pairs = p.get<MooseEnumItem, MooseEnumItem>("first", "second");

  for (int i = 0; i < 4; ++i)
  {
    EXPECT_EQ(pairs[i].first, v1[i]);
    EXPECT_EQ(pairs[i].second, v2[i]);
  }
}

TEST(InputParameters, getPairLength)
{
  std::vector<std::string> num_words{"zero", "one", "two"};

  InputParameters p = emptyInputParameters();
  p.addParam<std::vector<std::string>>("first", num_words, "");
  p.addParam<std::vector<int>>("second", std::vector<int>{0, 1, 2, 3}, "");

  try
  {
    auto pairs = p.get<std::string, int>("first", "second");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "Vector parameters first:(size: 3) and second:(size: 4) are of different lengths") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(InputParameters, getControllablePairs)
{
  std::vector<std::string> num_words{"zero", "one", "two", "three"};

  InputParameters p = emptyInputParameters();
  p.addParam<std::vector<std::string>>("first", num_words, "");
  p.addParam<std::vector<int>>("second", std::vector<int>{0, 1, 2, 3}, "");
  p.declareControllable("first");

  try
  {
    auto pairs = p.get<std::string, int>("first", "second");
    FAIL() << "Missing expected exception.";
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(
        msg.find(
            "first: and/or second: are controllable parameters and cannot be "
            "retireved using the MooseObject::getParam/InputParameters::get methods for pairs") !=
        std::string::npos)
        << "Failed with unexpected error message: " << msg;
  }
}

TEST(InputParameters, transferParameters)
{
  // Add parameters of various types to p1
  InputParameters p1 = emptyInputParameters();
  p1.addRequiredParam<Real>("r1", "Required 1");
  p1.addRequiredRangeCheckedParam<Real>("r2", "r2 > 0", "Required 2");
  p1.addRequiredCoupledVar("r3", "Required 3");
  MooseEnum options("option1 option2");
  MultiMooseEnum several_options("option1 option2");
  p1.addRequiredParam<MooseEnum>("r4", options, "Required 4");
  p1.addRequiredParam<MultiMooseEnum>("r5", several_options, "Required 5");
  p1.addParam<Real>("o1", 3, "Optional 1");
  p1.addRangeCheckedParam<Real>("o2", "o2 > 0", "Optional 2");
  p1.addCoupledVar("o3", "Optional 3");
  p1.addParam<MooseEnum>("o4", options, "Optional 4");
  p1.addParam<MultiMooseEnum>("o5", several_options, "Optional 5");
  p1.addParam<Real>("od1", "Optional with default 1");
  p1.addRangeCheckedParam<Real>("od2", "od2 > 0", "Optional with default 2");
  p1.addCoupledVar("od3a", 1., "Optional with default 3a");
  p1.addCoupledVar("od3b", {1., 2.}, "Optional with default 3b");

  // Params with modified attributes
  p1.declareControllable("r1");
  p1.addPrivateParam<Real>("private", 2);

  // Transfer them all to p2
  InputParameters p2 = emptyInputParameters();
  p2.transferParam<Real>(p1, "r1");
  p2.transferParam<Real>(p1, "r2");
  p2.transferParam<std::vector<VariableName>>(p1, "r3");
  p2.transferParam<MooseEnum>(p1, "r4");
  p2.transferParam<MultiMooseEnum>(p1, "r5");
  p2.transferParam<Real>(p1, "o1");
  p2.transferParam<Real>(p1, "o2");
  p2.transferParam<std::vector<VariableName>>(p1, "o3");
  p2.transferParam<MooseEnum>(p1, "o4");
  p2.transferParam<MultiMooseEnum>(p1, "o5");
  p2.transferParam<Real>(p1, "od1");
  p2.transferParam<Real>(p1, "od2");
  p2.transferParam<std::vector<VariableName>>(p1, "od3a");
  p2.transferParam<std::vector<VariableName>>(p1, "od3b");
  p2.transferParam<Real>(p1, "private");

  // Check that all the parameters match
  // Same parameters
  EXPECT_EQ(p1.have_parameter<Real>("r1"), p2.have_parameter<Real>("r1"));
  EXPECT_EQ(p1.have_parameter<Real>("r2"), p2.have_parameter<Real>("r2"));
  EXPECT_EQ(p1.hasCoupledValue("r3"), p2.hasCoupledValue("r3"));
  EXPECT_EQ(p1.have_parameter<MooseEnum>("r4"), p2.have_parameter<MooseEnum>("r4"));
  EXPECT_EQ(p1.have_parameter<MultiMooseEnum>("r5"), p2.have_parameter<MultiMooseEnum>("r5"));
  EXPECT_EQ(p1.have_parameter<Real>("o1"), p2.have_parameter<Real>("o1"));
  EXPECT_EQ(p1.have_parameter<Real>("o2"), p2.have_parameter<Real>("o2"));
  EXPECT_EQ(p1.hasCoupledValue("o3"), p2.hasCoupledValue("o3"));
  EXPECT_EQ(p1.have_parameter<MooseEnum>("o4"), p2.have_parameter<MooseEnum>("o4"));
  EXPECT_EQ(p1.have_parameter<MultiMooseEnum>("o5"), p2.have_parameter<MultiMooseEnum>("o5"));
  EXPECT_EQ(p1.have_parameter<Real>("od1"), p2.have_parameter<Real>("od1"));
  EXPECT_EQ(p1.have_parameter<Real>("od2"), p2.have_parameter<Real>("od2"));
  EXPECT_EQ(p1.hasDefaultCoupledValue("od3a"), p2.hasDefaultCoupledValue("od3a"));
  EXPECT_EQ(p1.hasDefaultCoupledValue("od3b"), p2.hasDefaultCoupledValue("od3b"));

  // Same required status
  EXPECT_EQ(p1.isParamRequired("r1"), p2.isParamRequired("r1"));
  EXPECT_EQ(p1.isParamRequired("r2"), p2.isParamRequired("r2"));
  EXPECT_EQ(p1.isParamRequired("r3"), p2.isParamRequired("r3"));
  EXPECT_EQ(p1.isParamRequired("r4"), p2.isParamRequired("r4"));
  EXPECT_EQ(p1.isParamRequired("r5"), p2.isParamRequired("r5"));
  EXPECT_EQ(p1.isParamRequired("o1"), p2.isParamRequired("o1"));
  EXPECT_EQ(p1.isParamRequired("o2"), p2.isParamRequired("o2"));
  EXPECT_EQ(p1.isParamRequired("o3"), p2.isParamRequired("o3"));
  EXPECT_EQ(p1.isParamRequired("o4"), p2.isParamRequired("o4"));
  EXPECT_EQ(p1.isParamRequired("o5"), p2.isParamRequired("o5"));
  EXPECT_EQ(p1.isParamRequired("od1"), p2.isParamRequired("od1"));
  EXPECT_EQ(p1.isParamRequired("od2"), p2.isParamRequired("od2"));
  EXPECT_EQ(p1.isParamRequired("od3a"), p2.isParamRequired("od3a"));
  EXPECT_EQ(p1.isParamRequired("od3b"), p2.isParamRequired("od3b"));

  // Same private status
  EXPECT_EQ(p1.isPrivate("r1"), p2.isPrivate("r1"));
  EXPECT_EQ(p1.isPrivate("private"), p2.isPrivate("private"));

  // Same controllable status
  EXPECT_EQ(p1.isControllable("r1"), p2.isControllable("r1"));
  EXPECT_EQ(p1.isControllable("r2"), p2.isControllable("r2"));

  // Same defaults
  EXPECT_EQ(p1.get<Real>("od1"), p2.get<Real>("od1"));
  EXPECT_EQ(p1.get<Real>("od2"), p2.get<Real>("od2"));
  EXPECT_EQ(p1.defaultCoupledValue("od3a"), p2.defaultCoupledValue("od3a"));
  // EXPECT_EQ(p1.defaultCoupledValue("od3b", 1), p2.defaultCoupledValue("od3b", 1));

  // Same docstrings
  EXPECT_EQ(p1.getDocString("r1"), p2.getDocString("r1"));
  EXPECT_EQ(p1.getDocString("r2"), p2.getDocString("r2"));
  EXPECT_EQ(p1.getDocString("r3"), p2.getDocString("r3"));
  EXPECT_EQ(p1.getDocString("r4"), p2.getDocString("r4"));
  EXPECT_EQ(p1.getDocString("r5"), p2.getDocString("r5"));
  EXPECT_EQ(p1.getDocString("o1"), p2.getDocString("o1"));
  EXPECT_EQ(p1.getDocString("o2"), p2.getDocString("o2"));
  EXPECT_EQ(p1.getDocString("o3"), p2.getDocString("o3"));
  EXPECT_EQ(p1.getDocString("o4"), p2.getDocString("o4"));
  EXPECT_EQ(p1.getDocString("o5"), p2.getDocString("o5"));
  EXPECT_EQ(p1.getDocString("od1"), p2.getDocString("od1"));
  EXPECT_EQ(p1.getDocString("od2"), p2.getDocString("od2"));
  EXPECT_EQ(p1.getDocString("od3a"), p2.getDocString("od3a"));
  EXPECT_EQ(p1.getDocString("od3b"), p2.getDocString("od3b"));
}
