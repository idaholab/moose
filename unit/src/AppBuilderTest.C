//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "AppBuilder.h"
#include "MooseMain.h"
#include "MooseApp.h"
#include "AppFactory.h"
#include "CommandLine.h"

TEST(AppBuilderTest, buildParamsErrors)
{
  const auto test = [](std::vector<std::string> args, const std::string & expected_error)
  {
    auto parser = std::make_shared<Parser>(std::vector<std::string>());
    parser->parse();
    Moose::AppBuilder app_builder(parser);

    // Yes, this is very dirty. And I think it's illegal.
    args.insert(args.begin(), "/path/to/exe");
    std::vector<char *> argv;
    argv.reserve(args.size());
    std::transform(args.begin(),
                   args.end(),
                   std::back_inserter(argv),
                   [](std::string & s) { return s.data(); });

    try
    {
      app_builder.buildParams("UnusedAppType", "main", argv.size(), argv.data(), MPI_COMM_WORLD);
      FAIL();
    }
    catch (const std::exception & err)
    {
      EXPECT_EQ(std::string(err.what()), expected_error);
    }
  };

  test({"Application/app=foo"},
       "The command-line input option Application/app is not supported. Please use "
       "Application/type= or --type instead.");
  test({"Application/type=foo", "--type", "foo"},
       "You cannot specify --type and Application/type together on the command line");
  test({"--type=foo", "--app", "bar"},
       "You cannot specify the command-line options --type and --app together.");
}

TEST(AppBuilderTest, buildParamsTypes)
{
  const auto test = [](std::vector<std::string> args,
                       const std::string & expected_type,
                       const std::optional<std::string> input_contents = {})
  {
    std::shared_ptr<Parser> parser;
    if (input_contents)
      parser = std::make_shared<Parser>("foo.i", input_contents);
    else
      parser = std::make_shared<Parser>(std::vector<std::string>());
    parser->parse();
    Moose::AppBuilder app_builder(parser);

    // Yes, this is very dirty. And I think it's illegal.
    args.insert(args.begin(), "/path/to/exe");
    std::vector<char *> argv;
    argv.reserve(args.size());
    std::transform(args.begin(),
                   args.end(),
                   std::back_inserter(argv),
                   [](std::string & s) { return s.data(); });

    try
    {
      app_builder.buildParams("DefaultApp", "main", argv.size(), argv.data(), MPI_COMM_WORLD);
      FAIL();
    }
    catch (const std::exception & err)
    {
      EXPECT_EQ(std::string(err.what()),
                std::string("The application type '" + expected_type + "' is not registered."));
    }
  };

  test({}, "DefaultApp");
  test({}, "AppFromInput", "Application/type=AppFromInput");
  test({"--type=AppFromType"}, "AppFromType");
  test({"--app=AppFromApp"}, "AppFromApp");
}

TEST(AppBuilderTest, buildParamsFromCommandLineErrors)
{
  auto params = MooseApp::validParams();

  const auto test = [](const std::vector<std::string> & args, const std::string & error)
  {
    auto command_line = std::make_shared<CommandLine>();
    command_line->addArguments(args);
    command_line->parse();

    auto params = MooseApp::validParams();
    params.set<std::shared_ptr<CommandLine>>("_command_line") = command_line;

    auto parser = std::make_shared<Parser>(std::vector<std::string>());
    parser->parse();

    Moose::AppBuilder app_builder(parser);

    try
    {
      app_builder.buildParamsFromCommandLine("main", params, MPI_COMM_WORLD);
      FAIL();
    }
    catch (const std::exception & err)
    {
      EXPECT_EQ(std::string(err.what()), error);
    }
  };

  test({"Application/mesh_only=foo", "--mesh-only"},
       "The command-line option '--mesh-only' and the command-line parameter "
       "'Application/mesh_only' apply to the same value and cannot be set together.");
  test({"Application/input_file=abcd"},
       "The command-line parameter 'Application/input_file=' cannot be used. Use the command-line "
       "option '-i <input file(s)> instead.");
}
