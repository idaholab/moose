//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "hit/hit.h"

#include "ParameterExtraction.h"
#include "InputParameters.h"
#include "ParseUtils.h"
#include "MooseUtils.h"
#include "MooseBase.h"

std::unique_ptr<const hit::Node>
buildTestRoot(const std::vector<std::string> & params)
{
  return std::unique_ptr<const hit::Node>(hit::parse("file", MooseUtils::stringJoin(params, "\n")));
}

using Moose::ParameterExtraction::extract;

/**
 * Test that a parameter ignored with ignoreParameter is not extracted
 */
TEST(ParameterExtractionTest, extractIgnored)
{
  const auto setup_params = []()
  {
    auto params = emptyInputParameters();
    params.addParam<bool>("foo", "doc");
    return params;
  };
  const auto setup_root = []() { return buildTestRoot({"Section/foo=true"}); };

  // Not ignored, should be set
  {
    auto params = setup_params();
    const auto root = setup_root();
    extract(*root, "Section", params);
    ASSERT_TRUE(params.isParamSetByUser("foo"));
  }
  // Ignored, should not be set
  {
    auto params = setup_params();
    params.ignoreParameter<bool>("foo");
    const auto root = setup_root();
    extract(*root, "Section", params);
    ASSERT_FALSE(params.isParamSetByUser("foo"));
  }
}

/**
 * Test that extracted parameters have the hit node set
 */
TEST(ParameterExtractionTest, extractSetHitNode)
{
  auto params = emptyInputParameters();
  params.addParam<bool>("foo", "doc");
  const auto root = buildTestRoot({"Section/foo=true"});
  const auto node = root->find("Section/foo");
  const auto info = extract(*root, "Section", params);
  ASSERT_NE(node, nullptr);
  ASSERT_TRUE(info.errors.empty());
  ASSERT_EQ(params.getHitNode("foo"), node);
}

/**
 * Test that extracted variables are marked as extracted
 */
TEST(ParameterExtractionTest, extractSetExtractedVariables)
{
  auto params = emptyInputParameters();
  params.addParam<bool>("foo", "doc");
  params.addParam<bool>("bar", "doc");
  const auto root =
      buildTestRoot({"GlobalParams/foo=true", "Section/bar=true", "Section/unused=abcd"});
  ASSERT_NE(root->find("Section/unused"), nullptr);
  const auto info = extract(*root, "Section", params);
  ASSERT_TRUE(info.errors.empty());
  const auto & vars = info.extracted_variables;
  ASSERT_NE(std::find(vars.begin(), vars.end(), "GlobalParams/foo"), vars.end());
  ASSERT_NE(std::find(vars.begin(), vars.end(), "Section/bar"), vars.end());
  ASSERT_EQ(std::find(vars.begin(), vars.end(), "Section/unused"), vars.end());
}

/**
 * Test that a deprecated parameter will report as such
 */
TEST(ParameterExtractionTest, extractSetDeprecated)
{
  const auto setup_params = []()
  {
    auto params = emptyInputParameters();
    params.addDeprecatedParam<bool>("foo", false, "doc", "dep_message");
    return params;
  };
  const auto setup_root = []() { return buildTestRoot({"Section/foo=true"}); };

  // No object type set, so deprecated key doesn't have the object
  {
    auto params = setup_params();
    const auto root = setup_root();
    const auto info = extract(*root, "Section", params);
    ASSERT_TRUE(info.errors.empty());
    ASSERT_EQ(info.deprecated_params.size(), 1);
    const auto it = info.deprecated_params.find("foo");
    ASSERT_TRUE(it != info.deprecated_params.end());
    ASSERT_EQ(it->second,
              "file:1.1:\nSection/foo: The parameter 'foo' is deprecated.\ndep_message");
    ASSERT_TRUE(params.get<bool>("foo"));
  }

  // Object type set, so deprecated key has the object prefix
  {
    auto params = setup_params();
    params.set<std::string>(MooseBase::type_param) = "type";
    const auto root = setup_root();
    const auto info = extract(*root, "Section", params);
    ASSERT_TRUE(info.errors.empty());
    ASSERT_EQ(info.deprecated_params.size(), 1);
    const auto it = info.deprecated_params.find("type_foo");
    ASSERT_TRUE(it != info.deprecated_params.end());
    ASSERT_EQ(it->second,
              "file:1.1:\nSection/foo: The parameter 'foo' is deprecated.\ndep_message");
    ASSERT_TRUE(params.get<bool>("foo"));
  }

  // Don't report deprecated for global params (are we sure we want to do this?)
  {
    auto params = setup_params();
    params.set<std::string>(MooseBase::type_param) = "type";
    const auto root = buildTestRoot({"GlobalParams/foo=true"});
    const auto info = extract(*root, "Section", params);
    ASSERT_TRUE(info.errors.empty());
    ASSERT_TRUE(info.deprecated_params.empty());
    ASSERT_TRUE(params.get<bool>("foo"));
  }
}

/**
 * Test that a parameter in [GlobalParams] will set a value
 */
TEST(ParameterExtractionTest, extractGlobal)
{
  auto params = emptyInputParameters();
  params.addParam<bool>("foo", false, "doc");
  const auto root = buildTestRoot({"GlobalParams/foo=true"});
  const auto node = root->find("GlobalParams/foo");
  ASSERT_NE(node, nullptr);
  const auto info = extract(*root, "Section", params);
  ASSERT_TRUE(info.errors.empty());
  ASSERT_TRUE(params.isParamSetByUser("foo"));
  ASSERT_EQ(params.getHitNode("foo"), node);
  ASSERT_TRUE(Moose::ParseUtils::isGlobal(*node));
  ASSERT_TRUE(params.get<bool>("foo"));
}

/**
 * Test that exceptions in setting are caught
 */
TEST(ParameterExtractionTest, extractSetCatch)
{
  auto params = emptyInputParameters();
  params.addParam<bool>("foo", "doc");
  const auto root = buildTestRoot({"Section/foo=a"});
  const auto node = root->find("Section/foo");
  ASSERT_NE(node, nullptr);
  const auto info = extract(*root, "Section", params);
  ASSERT_EQ(info.errors.size(), 1);
  ASSERT_EQ(info.errors[0].message, "invalid boolean syntax for parameter: foo='a'");
  ASSERT_EQ(info.errors[0].node, node);
}

/**
 * Test that command line parameters are set as set
 * via InputParameters::commandLineParamSet
 */
TEST(ParameterExtractionTest, extractCommandLineParameter)
{
  auto params = emptyInputParameters();
  params.registerBase("Application"); // required for command line params
  params.addCommandLineParam<bool>("foo", "--foo", "doc");
  params.enableInputCommandLineParam("foo");
  const auto root = buildTestRoot({"Section/foo=true"});
  const auto node = root->find("Section/foo");
  ASSERT_NE(node, nullptr);
  const auto info = extract(*root, "Section", params);
  ASSERT_TRUE(info.errors.empty());
  ASSERT_TRUE(params.isParamSetByUser("foo"));
  ASSERT_TRUE(params.get<bool>("foo"));
  ASSERT_EQ(params.getHitNode("foo"), node);
}

/**
 * Test that std::vector<VariableName> parameters are
 * setup appropriately with default values
 */
TEST(ParameterExtractionTest, extractVectorVariableName)
{
  auto params = emptyInputParameters();
  params.addParam<std::vector<VariableName>>("names", "doc");
  const auto root = buildTestRoot({"Section/names=\"1\""});
  const auto node = root->find("Section/names");
  ASSERT_NE(node, nullptr);
  const auto info = extract(*root, "Section", params);
  ASSERT_TRUE(info.errors.empty());
  ASSERT_TRUE(params.isParamSetByUser("names"));
  ASSERT_TRUE(params.get<std::vector<VariableName>>("names").empty());
  ASSERT_EQ(params.numberDefaultCoupledValues("names"), 1);
  ASSERT_EQ(params.defaultCoupledValue("names"), 1);
}

/**
 * Test that std::vector<VariableName> parameter errors
 * are appropriately caught
 */
TEST(ParameterExtractionTest, extractVectorVariableNameError)
{
  auto params = emptyInputParameters();
  params.addParam<std::vector<VariableName>>("names", "doc");
  const auto root = buildTestRoot({"Section/names=\"1 a\""});
  const auto node = root->find("Section/names");
  ASSERT_NE(node, nullptr);
  const auto info = extract(*root, "Section", params);
  ASSERT_EQ(info.errors.size(), 1);
  ASSERT_EQ(info.errors[0].message,
            "invalid value for 'Section/names': coupled vectors where some parameters are reals "
            "and others are variables are not supported");
  ASSERT_EQ(info.errors[0].node, node);
}

/**
 * Test that range checked parameters are properly checked
 * and errors are caught
 */
TEST(ParameterExtractionTest, extractRangeChecked)
{
  const auto setup_params = []()
  {
    auto params = emptyInputParameters();
    params.addRangeCheckedParam<Real>("foo", "foo > 1", "doc");
    return params;
  };

  // success
  {
    auto params = setup_params();
    const auto root = buildTestRoot({"Section/foo=2"});
    const auto info = extract(*root, "Section", params);
    ASSERT_TRUE(info.errors.empty());
  }
  // failed
  {
    auto params = setup_params();
    const auto root = buildTestRoot({"Section/foo=0"});
    const auto node = root->find("Section/foo");
    ASSERT_NE(node, nullptr);
    const auto info = extract(*root, "Section", params);
    ASSERT_EQ(info.errors.size(), 1);
    ASSERT_EQ(info.errors[0].message,
              "Range check failed for parameter Section/foo; expression = 'foo > 1', value = 0");
    ASSERT_EQ(info.errors[0].node, node);
  }
}

/**
 * Test that a private parameter is not set an error is thrown when the
 * field does not come from a global value
 */
TEST(ParameterExtractionTest, extractPrivate)
{
  const auto setup_params = []()
  {
    auto params = emptyInputParameters();
    params.addPrivateParam<bool>("foo");
    return params;
  };

  // Set directly, is an error
  {
    auto params = setup_params();
    const auto root = buildTestRoot({"Section/foo=true"});
    const auto node = root->find("Section/foo");
    ASSERT_NE(node, nullptr);
    const auto info = extract(*root, "Section", params);
    ASSERT_EQ(info.errors.size(), 1);
    ASSERT_EQ(info.errors[0].message, "parameter 'Section/foo' is private and cannot be set");
    ASSERT_EQ(info.errors[0].node, node);
    ASSERT_FALSE(params.isParamSetByUser("foo"));
  }
  // Set in global, is just a skip
  {
    auto params = setup_params();
    const auto root = buildTestRoot({"GlobalParams/foo=true"});
    const auto info = extract(*root, "Section", params);
    ASSERT_TRUE(info.errors.empty());
    ASSERT_FALSE(params.isParamSetByUser("foo"));
  }
}

/**
 * Test extracting parameters with aliases
 */
TEST(ParameterExtractionTest, extractAliases)
{
  const auto setup_params = []()
  {
    auto params = emptyInputParameters();
    params.addParam<bool>("foo", "doc");
    params.renameParam("foo", "new_foo", "doc");
    return params;
  };

  // use old name
  {
    auto params = setup_params();
    const auto root = buildTestRoot({"Section/foo=true"});
    const auto info = extract(*root, "Section", params);
    ASSERT_TRUE(info.errors.empty());
    ASSERT_TRUE(params.get<bool>("new_foo"));
  }
  // use new name
  {
    auto params = setup_params();
    const auto root = buildTestRoot({"Section/new_foo=true"});
    const auto info = extract(*root, "Section", params);
    ASSERT_TRUE(info.errors.empty());
    ASSERT_TRUE(params.get<bool>("new_foo"));
  }
  // prefer new name, and old is not extracted (will result in unused var error)
  {
    auto params = setup_params();
    const auto root = buildTestRoot({"Section/foo=false", "Section/new_foo=true"});
    const auto foo_node = root->find("Section/foo");
    ASSERT_NE(foo_node, nullptr);
    const auto new_foo_node = root->find("Section/new_foo");
    ASSERT_NE(new_foo_node, nullptr);
    const auto info = extract(*root, "Section", params);
    ASSERT_TRUE(info.errors.empty());
    ASSERT_EQ(info.extracted_variables.size(), 1);
    ASSERT_EQ(info.extracted_variables[0], "Section/new_foo");
    ASSERT_TRUE(params.get<bool>("new_foo"));
    ASSERT_EQ(params.getHitNode("new_foo"), new_foo_node);
  }
}

/**
 * Test that a merged command line parameter gets the correct context
 * (see idaholab/moose#31461)
 */
TEST(ParameterExtractionTest, extractFromMergedCommandLine)
{
  // success
  {
    auto params = emptyInputParameters();
    params.addParam<std::string>("foo", "doc");
    const std::unique_ptr<hit::Node> root(hit::parse("file", "Section/foo=frominput\n"));
    const std::unique_ptr<hit::Node> cli_root(
        hit::parse(Moose::hit_command_line_filename, "Section/foo=fromcli\n"));
    hit::merge(cli_root.get(), root.get());
    const auto info = extract(*root, cli_root.get(), "Section", params);
    ASSERT_TRUE(info.errors.empty());
    ASSERT_EQ(params.get<std::string>("foo"), "fromcli");
    const auto node = cli_root->find("Section/foo");
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(params.getHitNode("foo"), node);
  }

  // error, associated with the cli node
  {
    auto params = emptyInputParameters();
    params.addParam<bool>("foo", "doc");
    const std::unique_ptr<hit::Node> root(hit::parse("file", "Section/foo=true\n"));
    const std::unique_ptr<hit::Node> cli_root(
        hit::parse(Moose::hit_command_line_filename, "Section/foo=abcd\n"));
    hit::merge(cli_root.get(), root.get());
    const auto info = extract(*root, cli_root.get(), "Section", params);
    ASSERT_EQ(info.errors.size(), 1);
    const auto node = cli_root->find("Section/foo");
    ASSERT_NE(node, nullptr);
    ASSERT_EQ(info.errors[0].node, node);
  }
}
