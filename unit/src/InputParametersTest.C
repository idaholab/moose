//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "InputParameters.h"
#include "MooseEnum.h"
#include "MultiMooseEnum.h"
#include "Conversion.h"
#include "Parser.h"
#include "MooseUnitUtils.h"
#include "MooseBase.h"

TEST(InputParametersTest, checkControlParamPrivateError)
{
  InputParameters params = emptyInputParameters();
  params.addPrivateParam<Real>("private", 1);
  params.declareControllable("private");
  Moose::UnitUtils::assertThrows<MooseRuntimeError>([&params]() { params.checkParams(""); },
                                                    "private parameter '' marked controllable");
}

TEST(InputParametersTest, checkRangeCheckedParam)
{
  // This tests for the bug https://github.com/idaholab/moose/issues/8586.
  // It makes sure that range-checked input file parameters comparison functions
  // do absolute floating point comparisons instead of using a default epsilon.
  InputParameters params = emptyInputParameters();
  params.addRangeCheckedParam<Real>("p", 1.000000000000001, "p = 1", "Some doc");
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.checkParams(""); },
      "p: Range check failed; expression = 'p = 1', value = 1");

  const auto test_vector_error = [](const std::vector<Real> & value,
                                    const std::string & range_function,
                                    const bool is_user_error,
                                    const std::string expect_error)
  {
    InputParameters params = emptyInputParameters();
    const std::string name = "p";
    params.addRangeCheckedParam<std::vector<Real>>(name, value, range_function, "Some doc");
    const auto param = dynamic_cast<libMesh::Parameters::Parameter<std::vector<Real>> *>(
        params.begin()->second.get());
    const auto result = params.rangeCheck<Real, Real>(name, name, *param);
    EXPECT_TRUE(result.has_value());
    ASSERT_EQ(is_user_error, result->first);
    ASSERT_EQ(expect_error, result->second);
  };

  // Invalid range function
  test_vector_error({}, "!", false, "Error parsing expression '!' for parameter p");
  // Check all values, vector has no values
  test_vector_error(
      {}, "p = 1", true, "Range checking empty vector parameter p; expression = 'p = 1'");
  // Check all values, invalid variable
  test_vector_error({1}, "a = 1", false, "Error parsing expression 'a = 1'");
  // Index check out of range
  test_vector_error(
      {1},
      "p_1 = 1",
      true,
      "Error parsing expression 'p_1 = 1' for parameter p; out of range variable 'p_1'");
  // Index check invalid variable
  test_vector_error(
      {1}, "p_a = 1", false, "Error parsing expression 'p_a = 1'; invalid variable 'p_a'");

  const auto test_scalar_error = [](const std::string & range_function,
                                    const bool is_user_error,
                                    const std::string expect_error)
  {
    InputParameters params = emptyInputParameters();
    const std::string name = "p";
    params.addRangeCheckedParam<Real>(name, 1, range_function, "Some doc");
    const auto param =
        dynamic_cast<libMesh::Parameters::Parameter<Real> *>(params.begin()->second.get());
    const auto result = params.rangeCheck<Real, Real>(name, name, *param);
    EXPECT_TRUE(result.has_value());
    ASSERT_EQ(is_user_error, result->first);
    ASSERT_EQ(expect_error, result->second);
  };

  // Invalid range function
  test_scalar_error("!", false, "Error parsing expression '!' for parameter p");
}

TEST(InputParametersTest, checkControlParamTypeError)
{
  InputParameters params = emptyInputParameters();
  params.addParam<PostprocessorName>("pp_name", "make_it_valid", "Some doc");
  params.declareControllable("pp_name");
  Moose::UnitUtils::assertThrows<MooseRuntimeError>([&params]() { params.checkParams(""); },
                                                    "non-controllable type");
}

TEST(InputParametersTest, checkControlParamValidError)
{
  InputParameters params = emptyInputParameters();
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.declareControllable("not_valid"); }, "cannot be marked as controllable");
}

TEST(InputParametersTest, checkSuppressedError)
{
  InputParameters params = emptyInputParameters();
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.suppressParameter<int>("nonexistent"); },
      "Unable to suppress nonexistent parameter");
}

TEST(InputParametersTest, checkSuppressingControllableParam)
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

TEST(InputParametersTest, checkSetDocString)
{
  InputParameters params = emptyInputParameters();
  params.addParam<Real>("little_guy", "What about that little guy?");
  params.setDocString("little_guy", "That little guy, I wouldn't worry about that little_guy.");
  ASSERT_TRUE(params.getDocString("little_guy")
                  .find("That little guy, I wouldn't worry about that little_guy.") !=
              std::string::npos)
      << "retrieved doc string has unexpected value '" << params.getDocString("little_guy") << "'";
}

TEST(InputParametersTest, checkSetDocStringError)
{
  InputParameters params = emptyInputParameters();
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]()
      {
        params.setDocString("little_guy",
                            "That little guy, I wouldn't worry about that little_guy.");
      },
      "Unable to set the documentation string (using setDocString)");
}

void
testBadParamName(const std::string & name)
{
  InputParameters params = emptyInputParameters();
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params, &name]() { params.addParam<bool>(name, "Doc"); }, "Invalid parameter name");
}

/// Just make sure we don't allow invalid parameter names
TEST(InputParametersTest, checkParamName)
{
  testBadParamName("p 0");
  testBadParamName("p-0");
  testBadParamName("p!0");
}

TEST(InputParametersTest, applyParameter)
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

  // applyParameter should throw error if input parameter doesn't match any valid parameters
  EXPECT_THROW(p1.applyParameter(p2, "invalid_param"), std::exception);
}

TEST(InputParametersTest, applyParametersVector)
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

TEST(InputParametersTest, applyParameters)
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

TEST(InputParametersTest, applyParametersPrivateOverride)
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

TEST(InputParametersTest, makeParamRequired)
{
  InputParameters params = emptyInputParameters();
  params.addParam<Real>("good_param", "documentation");
  params.addParam<Real>("wrong_param_type", "documentation");

  // Require existing parameter with the appropriate type
  EXPECT_FALSE(params.isParamRequired("good_param"));
  params.makeParamRequired<Real>("good_param");
  EXPECT_TRUE(params.isParamRequired("good_param"));

  // Require existing parameter with the wrong type
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.makeParamRequired<PostprocessorName>("wrong_param_type"); },
      "Unable to require nonexistent parameter: wrong_param_type");

  // Require non-existing parameter
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.makeParamRequired<PostprocessorName>("wrong_param_name"); },
      "Unable to require nonexistent parameter: wrong_param_name");
}

TEST(InputParametersTest, setPPandVofPP)
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

TEST(InputParametersTest, getPairs)
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

TEST(InputParametersTest, getMultiMooseEnumPairs)
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

TEST(InputParametersTest, getPairLength)
{
  std::vector<std::string> num_words{"zero", "one", "two"};

  InputParameters p = emptyInputParameters();
  p.addParam<std::vector<std::string>>("first", num_words, "");
  p.addParam<std::vector<int>>("second", std::vector<int>{0, 1, 2, 3}, "");

  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&p]() { p.get<std::string, int>("first", "second"); },
      "first: Vector parameters first(size: 3) and second(size: 4) are of different lengths");
}

TEST(InputParametersTest, getControllablePairs)
{
  std::vector<std::string> num_words{"zero", "one", "two", "three"};

  InputParameters p = emptyInputParameters();
  p.addParam<std::vector<std::string>>("first", num_words, "");
  p.addParam<std::vector<int>>("second", std::vector<int>{0, 1, 2, 3}, "");
  p.declareControllable("first");

  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&p]() { p.get<std::string, int>("first", "second"); },
      "Parameters first and/or second are controllable parameters and cannot be retireved using "
      "the MooseObject::getParam/InputParameters::get methods for pairs");
}

TEST(InputParametersTest, getParamList)
{
  std::vector<std::string> num_words{"zero", "one", "two", "three"};

  InputParameters p = emptyInputParameters();
  p.addParam<std::vector<std::string>>("first", num_words, "");
  p.addParam<std::vector<int>>("second", std::vector<int>{0, 1, 2, 3}, "");

  std::set<std::string> param_list({"first", "second"});
  EXPECT_EQ(p.getParametersList(), param_list);
}

TEST(InputParametersTest, transferParameters)
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

TEST(InputParametersTest, noDefaultValueError)
{
  InputParameters params = emptyInputParameters();
  params.addParam<Real>("dummy_real", "a dummy real");
  params.addParam<std::vector<Real>>("dummy_vector", "a dummy vector");

  // Throw an error message when no default value is provided
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.getParamHelper<Real>("dummy_real", params); },
      "The parameter \"dummy_real\" is being retrieved before being set.");

  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.getParamHelper<std::vector<Real>>("dummy_vector", params); },
      "The parameter \"dummy_vector\" is being retrieved before being set.");
}

TEST(InputParametersTest, fileNames)
{
  // Walker that kind-of does what the Builder does: walks parameters and sets input
  // parameters. In this case, we only care about the objects that we've declared
  // and their "file" parameter.
  //
  // I chose to do this like this because it means that these tests don't rely
  // on an application at all, so we're relying on the basic capability
  class ExtractWalker : public hit::Walker
  {
  public:
    ExtractWalker(std::map<std::string, std::unique_ptr<InputParameters>> & object_params)
      : _object_params(object_params)
    {
    }
    virtual void walk(const std::string &, const std::string &, hit::Node * n) override
    {
      if (auto it = _object_params.find(n->path()); it != _object_params.end())
      {
        auto & params = *it->second;
        params.setHitNode(*n, {});
        if (const auto node = n->find("file"))
        {
          params.setHitNode("file", *node, {});
          params.set_attributes("file", false);
          params.set<FileName>("file") = node->strVal();
        }
      }
    }

  private:
    std::map<std::string, std::unique_ptr<InputParameters>> & _object_params;
  };

  // Runs a test.
  // inputs: map of filename -> input text
  // expected_values: map of object name -> the relative path we expect
  // cli_args: any cli args to add
  auto run_test = [](const std::map<std::string, std::string> & inputs,
                     const std::map<std::string, std::string> & expected_values,
                     const std::vector<std::string> & cli_args = {})
  {
    std::vector<std::string> filenames, input_text;
    for (const auto & [filename, text] : inputs)
    {
      filenames.push_back(filename);
      input_text.push_back(text);
    }

    // Parse the inputs
    Parser parser(filenames, input_text);
    parser.setCommandLineParams(cli_args);
    parser.parse();

    // Define the parameters that we care about
    InputParameters base_params = emptyInputParameters();
    base_params.addParam<FileName>("file", "file name");

    // Initialize the params for each object
    std::map<std::string, std::unique_ptr<InputParameters>> object_params;
    for (const auto & object_values_pair : expected_values)
    {
      const auto & object_name = object_values_pair.first;
      auto params = std::make_unique<InputParameters>(base_params);
      object_params.emplace(object_name, std::move(params));
    }

    // Fill the object parameters from the input
    ExtractWalker ew(object_params);
    parser.getRoot().walk(&ew, hit::NodeType::Section);

    // Finalize the parameters, which sets the filep aths
    for (auto & name_params_pair : object_params)
      name_params_pair.second->finalize("");

    // Check all of the values that we expect
    for (const auto & [object_name, expected_value] : expected_values)
    {
      const auto & params = *object_params.at(object_name);
      const std::string file = params.get<FileName>("file");
      const std::filesystem::path file_path(file);

      if (file.size())
      {
        EXPECT_TRUE(file_path.is_absolute());
      }
      if (expected_value.size())
      {
        const auto relative = std::string(std::filesystem::proximate(file));
        EXPECT_EQ(relative, expected_value);
      }
      else
      {
        EXPECT_TRUE(file.empty());
      }
    }
  };

  const auto include_files_path = std::filesystem::path(std::string(__FILE__)).parent_path() /
                                  std::filesystem::path("../files/InputParametersTest");
  const auto include_file = [&include_files_path](const auto & filename)
  { return std::string((include_files_path / std::filesystem::path(filename)).c_str()); };
  const auto relative_include_file = [&include_files_path](const auto & filename)
  {
    return std::string(
        std::filesystem::proximate(include_files_path / std::filesystem::path(filename)).c_str());
  };

  // const std::filesystem
  // One input file, same directory
  // main.i:
  // [object1]
  //   file = value
  // []
  //
  // object1:
  //   file: ./value
  run_test({{"main.i", "object1/file=value"}}, {{"object1", "value"}});

  // Three input files: cwd, behind a directory, and ahead a directory
  // main.i:
  //   main/file=main
  // ../behind.i:
  //   behind/file=behind
  // ahead/ahead.i:
  //   ahead/file=ahead
  //
  // main: ./main
  // behind: ../behind/behind
  // ahead: ./ahead/ahead
  run_test({{"main.i", "main/file=main"},
            {"../behind.i", "behind/file=behind"},
            {"ahead/ahead.i", "ahead/file=ahead"}},
           {{"main", "main"}, {"behind", "../behind"}, {"ahead", "ahead/ahead"}});

  // Two input files in different directories, with no file parameters. But, one
  // of the input files (in a different directory) has the section only for the object
  // that takes file parameters. The file is then applied via command line arguments
  // and the context should be taken from the parent section (defined in the input
  // in another folder)
  // main.i:
  //   unused=unused
  // ./ahead/ahead.i:
  //   [object]
  //   []
  // CLI args:
  //   object/file=foo
  //
  // object: ./ahead/foo
  run_test({{"main.i", "unused=unused"}, {"./ahead/ahead.i", "[object][]"}},
           {{"object", "ahead/foo"}},
           {"object/file=foo"});

  // Single input in a different directory with no contents, but will store the root
  // in a different folder and all files should be applied to that folder
  // ../../main.i:
  //   unused=unused
  // CLI args:
  //   object/file=foo
  //
  // object: ../../foo
  run_test({{"../../main.i", "unused=unused"}}, {{"object", "../../foo"}}, {"object/file=foo"});

  // Input with an include in a different directory
  run_test({{"../main.i", "!include " + include_file("simple_input.i")}},
           {{"object", relative_include_file("../foo")}});
}

TEST(InputParametersTest, alphaCommandLineParamSwitch)
{
  InputParameters params = emptyInputParameters();
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.addCommandLineParam<bool>("1value", "--1value", "Doc"); },
      "The switch '--1value' for the command line parameter '1value' is invalid. It must begin "
      "with an alphabetical character.");
}

TEST(InputParametersTest, setGlobalCommandLineParamNotCLParam)
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::string>("param", "Doc");
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.setGlobalCommandLineParam("param"); },
      "InputParameters::setGlobalCommandLineParam: The parameter 'param' is not a command line "
      "parameter");
}

TEST(InputParametersTest, getCommandLineMetadataNotCLParam)
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::string>("param", "Doc");
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.getCommandLineMetadata("param"); },
      "InputParameters::getCommandLineMetadata: The parameter 'param' is not a command line "
      "parameter");
}

TEST(InputParametersTest, commandLineParamSetNotCLParam)
{
  InputParameters params = emptyInputParameters();
  params.addParam<std::string>("param", "Doc");
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&params]() { params.commandLineParamSet("param", {}); },
      "InputParameters::commandLineParamSet: The parameter 'param' is not a command line "
      "parameter");
}

TEST(InputParametersTest, isCommandLineParameter)
{
  InputParameters params = emptyInputParameters();
  params.addCommandLineParam<std::string>("cliparam", "--cliparam", "Doc");
  params.addParam<std::string>("noncliparam", "Doc");

  EXPECT_TRUE(params.isCommandLineParameter("cliparam"));
  EXPECT_FALSE(params.isCommandLineParameter("noncliparam"));
}

TEST(InputParametersTest, commandLineParamKnownArg)
{
  // Helper for checking if a command line name is registered
  const auto have_cl_name = [](const std::string & var) -> bool
  {
    const auto names = libMesh::command_line_names();
    return std::find(names.begin(), names.end(), var) != names.end();
  };

  const std::vector<std::string> switches{"-iptest_knownarg", "--iptest_anotherarg"};

  // These arguments should not be recognized yet; these need to be unique
  // names because the libMesh command line is shared across unit tests
  for (const auto & v : switches)
    ASSERT_FALSE(have_cl_name(v));

  // Adding these arguments should call back to libMesh::add_command_line_name
  InputParameters params = emptyInputParameters();
  const auto combined_switches = MooseUtils::stringJoin(switches, " ");
  params.addCommandLineParam<std::string>("foo", combined_switches, "Doc");

  // Should now be recognized
  for (const auto & v : switches)
    ASSERT_TRUE(have_cl_name(v));
}

TEST(InputParametersTest, checkIsParamDefined)
{
  InputParameters params = emptyInputParameters();
  params.addParam<Real>("test_param", "This parameter should be indicated as being defined");
  ASSERT_TRUE(params.isParamDefined("test_param"));
  ASSERT_FALSE(params.isParamDefined("some_other_param"));
}

TEST(InputParametersTest, queryAndGetObjectType)
{
  InputParameters params = emptyInputParameters();

  // Not set
  ASSERT_EQ(params.queryObjectType(), nullptr);
  Moose::UnitUtils::assertThrows([&params]() { params.getObjectType(); },
                                 "InputParameters::getObjectType(): Missing '" +
                                     MooseBase::type_param + "' param",
                                 true);

  // Is set
  const auto object_type = "foo";
  params.set<std::string>(MooseBase::type_param) = object_type;
  const auto type_ptr = params.queryObjectType();
  ASSERT_NE(type_ptr, nullptr);
  ASSERT_EQ(*type_ptr, object_type);
  ASSERT_EQ(params.getObjectType(), *type_ptr);
}

TEST(InputParametersTest, dataFileParams)
{
  const std::string cwd = std::filesystem::current_path();
  const std::string moose_unit_data = std::filesystem::weakly_canonical(
      std::filesystem::path(__FILE__).parent_path() / std::filesystem::path("../data"));

  const std::string name = "test_param";
  const auto get_value = [&name](const auto value) -> std::string
  {
    using value_T = std::decay_t<decltype(value)>;
    InputParameters params = emptyInputParameters();
    params.addParam<value_T>(name, value, "doc");
    params.finalize("unused");
    return params.get<value_T>(name);
  };

  // relative path, becomes absolute with graceful despite not being found
  EXPECT_EQ(get_value(FileName("foo")), cwd + "/foo");
  // relative path, errors when not graceful
  EXPECT_MOOSEERROR_MSG_CONTAINS(get_value(DataFileName("foo")),
                                 name + ": Unable to find the data file 'foo'.");
  // absolute path, still absolute with graceful
  EXPECT_EQ(get_value(FileName("/foo")), "/foo");
  // absolute path, errors when not graceful
  EXPECT_MOOSEERROR_MSG(get_value(DataFileName("/foo")),
                        name + ": The absolute path '/foo' does not exist or is not readable.");
  // invalid explicit data name, same error with or without graceful
  {
    const std::string value = "bad:foo";
    const std::string message = name + ": Data from 'bad' is not registered to be searched";
    EXPECT_MOOSEERROR_MSG(get_value(FileName(value)), message);
    EXPECT_MOOSEERROR_MSG(get_value(DataFileName(value)), message);
  }
  // explicit data name, file doesn't exist, same error with or without graceful
  {
    const std::string value = "moose_unit:foo";
    const std::string message = name + ": The path 'foo' was not found in data from 'moose_unit'";
    EXPECT_MOOSEERROR_MSG(get_value(FileName(value)), message);
    EXPECT_MOOSEERROR_MSG(get_value(DataFileName(value)), message);
  }
  // explicit data name, file does exist; make sure this works for all
  // non-DataFileName types
  {
    const std::string value = "moose_unit:example_file";
    const std::string path = moose_unit_data + "/example_file";
    EXPECT_EQ(get_value(FileName(value)), path);
    EXPECT_EQ(get_value(FileNameNoExtension(value)), path);
    EXPECT_EQ(get_value(MeshFileName(value)), path);
    EXPECT_EQ(get_value(MatrixFileName(value)), path);
    EXPECT_EQ(get_value(DataFileName(value)), path);
  }
  // valid data name, not found when not searching data
  EXPECT_EQ(get_value(FileName("example_file")), cwd + "/example_file");
  // valid data name, found when searching with DataFileName
  EXPECT_EQ(get_value(DataFileName("example_file")), moose_unit_data + "/example_file");
  // can't use absolute paths when explicit with or without graceful
  {
    const std::string value = "moose_unit:/foo";
    const std::string message = name + ": Cannot use an absolute path";
    EXPECT_MOOSEERROR_MSG_CONTAINS(get_value(FileName(value)), message);
    EXPECT_MOOSEERROR_MSG_CONTAINS(get_value(DataFileName(value)), message);
  }
  // can't use ./ paths when explicit with or without graceful
  {
    const std::string value = "moose_unit:./foo";
    const std::string message = name + ": Cannot use a path that starts with './'";
    EXPECT_MOOSEERROR_MSG_CONTAINS(get_value(FileName(value)), message);
    EXPECT_MOOSEERROR_MSG_CONTAINS(get_value(DataFileName(value)), message);
  }
  // can't use out of dir paths when explicit with or without graceful
  {
    const std::string value = "moose_unit:../foo";
    const std::string message = name + ": Cannot use a relative path";
    EXPECT_MOOSEERROR_MSG_CONTAINS(get_value(FileName(value)), message);
    EXPECT_MOOSEERROR_MSG_CONTAINS(get_value(DataFileName(value)), message);
  }
}
