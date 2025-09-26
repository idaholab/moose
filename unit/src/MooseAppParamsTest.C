//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "AppFactory.h"
#include "MooseUnitUtils.h"
#include "Parser.h"
#include "CommandLine.h"
#include "ParseUtils.h"

#include "hit/hit.h"

std::unique_ptr<MooseApp>
createApp(const std::vector<std::string> & cli_args)
{
  return AppFactory::create("MooseUnitApp", cli_args);
};

TEST(MooseAppParamsTest, checkExclusiveParams)
{
  MOOSE_ASSERT_THROWS(
      MooseRuntimeError,
      createApp({"--check-input", "--recover"}),
      "--check-input: Cannot be used with parameter '--recover'; recover files might not exist");
  MOOSE_ASSERT_THROWS(
      MooseRuntimeError,
      createApp({"--test-restep", "--test-checkpoint-half-transient"}),
      "--test-restep: Cannot be used with parameter '--test-checkpoint-half-transient'");
  MOOSE_ASSERT_THROWS(MooseRuntimeError,
                      createApp({"--trap-fpe", "--no-trap-fpe"}),
                      "--trap-fpe: Cannot be used with parameter '--no-trap-fpe'");
}

TEST(MooseAppParamsTest, extractApplicationParams)
{
  struct ParseResult
  {
    std::unique_ptr<Parser> parser;
    std::unique_ptr<CommandLine> command_line;
  };
  const auto parse = [](const std::vector<std::string> & params,
                        const std::vector<std::string> & command_line_params) -> ParseResult
  {
    ParseResult parse_result;

    parse_result.command_line = std::make_unique<CommandLine>();
    parse_result.command_line->addArgument("app-opt");
    parse_result.command_line->addArguments(command_line_params);
    parse_result.command_line->parse();

    parse_result.parser = std::make_unique<Parser>("file", MooseUtils::stringJoin(params, "\n"));
    parse_result.parser->setCommandLineParams(parse_result.command_line->buildHitParams());
    parse_result.parser->parse();

    return parse_result;
  };

  // Success, set in both input and CLI args
  {
    auto parse_result = parse({"Application/error=true"}, {"Application/error_unused=true"});
    auto params = MooseApp::validParams();
    const auto & root = parse_result.parser->getRoot();
    const auto & cli_root = parse_result.parser->getCommandLineRoot();
    const auto error_node = root.find("Application/error", true);
    const auto error_unused_node = cli_root.find("Application/error_unused", true);

    MooseApp::extractApplicationParams(root, cli_root, params, false);

    ASSERT_EQ(params.get<bool>("error"), true);
    ASSERT_TRUE(params.isParamSetByUser("error"));
    ASSERT_EQ(params.getHitNode("error"), error_node);

    ASSERT_EQ(params.get<bool>("error_unused"), true);
    ASSERT_TRUE(params.isParamSetByUser("error_unused"));
    ASSERT_EQ(params.getHitNode("error_unused"), error_unused_node);

    auto & info_ptr = params.get<std::shared_ptr<const Moose::ParameterExtraction::ExtractionInfo>>(
        "_app_extraction_info");
    ASSERT_NE(info_ptr.get(), nullptr);
    auto & info = *info_ptr;
    ASSERT_EQ(info.errors.size(), 0);
    ASSERT_EQ(info.extracted_nodes.size(), 2);
    ASSERT_EQ(info.extracted_nodes[0], error_node);
    ASSERT_EQ(info.extracted_nodes[1], error_unused_node);
  }

  // Parse error without throw, so mooseError is thrown with combined message
  {
    auto parse_result = parse({"Application/error=foo"}, {"Application/error_unused=bar"});
    auto params = MooseApp::validParams();
    MOOSE_ASSERT_THROWS(
        MooseRuntimeError,
        MooseApp::extractApplicationParams(parse_result.parser->getRoot(),
                                           parse_result.parser->getCommandLineRoot(),
                                           params,
                                           false),
        "file:1.1: invalid boolean syntax for parameter: Application/error='foo'\n"
        "CLI_ARGS: invalid boolean syntax for parameter: Application/error_unused='bar'");
  }

  // Parse error with throw, so ParseError is thrown with message and node context
  {
    auto parse_result = parse({"Application/error=foo"}, {"Application/error_unused=bar"});
    auto params = MooseApp::validParams();
    auto error_node = parse_result.parser->getRoot().find("Application/error", true);
    auto error_unused_node =
        parse_result.parser->getCommandLineRoot().find("Application/error_unused", true);
    try
    {
      MooseApp::extractApplicationParams(
          parse_result.parser->getRoot(), parse_result.parser->getCommandLineRoot(), params, true);
      FAIL() << "Did not throw";
    }
    catch (const Moose::ParseUtils::ParseError & e)
    {
      ASSERT_EQ(e.error_messages.size(), 2);
      ASSERT_EQ(e.error_messages[0].node, error_node);
      ASSERT_EQ(e.error_messages[0].message,
                "invalid boolean syntax for parameter: Application/error='foo'");
      ASSERT_EQ(e.error_messages[1].node, error_unused_node);
      ASSERT_EQ(e.error_messages[1].message,
                "invalid boolean syntax for parameter: Application/error_unused='bar'");
    }
    catch (...)
    {
      FAIL() << "Did not throw Moose::ParseUtils::ParseError";
    }
  }
}
