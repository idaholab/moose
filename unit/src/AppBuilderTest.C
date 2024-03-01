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
                       const std::optional<std::string> input_contents = {},
                       const std::optional<std::string> deprecated_error = {})
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

    const auto deprecated_before = Moose::_deprecated_is_error;
    if (deprecated_error)
      Moose::_deprecated_is_error = true;

    try
    {
      app_builder.buildParams("DefaultApp", "main", argv.size(), argv.data(), MPI_COMM_WORLD);
      FAIL();
    }
    catch (const std::exception & err)
    {
      if (deprecated_error)
        EXPECT_TRUE(std::string(err.what()).find(*deprecated_error) != std::string::npos);
      else
        EXPECT_EQ(std::string(err.what()),
                  std::string("The application type '" + expected_type + "' is not registered."));
    }

    if (deprecated_error)
      Moose::_deprecated_is_error = deprecated_before;
  };

  test({}, "DefaultApp");
  test({}, "AppFromInput", "Application/type=AppFromInput");
  test({"--type=AppFromType"}, "AppFromType");
  test({"--app=AppFromApp"},
       "Unused",
       {},
       "The specified command line option '--app AppFromApp' is deprecated and will be removed in "
       "a future release.");
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

TEST(AppBuilderTest, extractParams)
{
  auto params = MooseApp::validParams();
  params.addParam<std::vector<std::string>>("string_values", "Doc1");
  params.addParam<bool>("bool_value", "Doc2");
  params.addParam<int>("int_value", "Doc3");
  params.addParam<unsigned int>("unsigned_int_value", "Doc4");

  const std::vector<std::string> string_values = {"abcd", "1234"};
  const bool bool_value = true;
  const int int_value = -100;

  auto command_line = std::make_unique<CommandLine>();
  command_line->addArgument("Application/bool_value=" + std::to_string(bool_value));
  command_line->parse();
  params.set<std::shared_ptr<CommandLine>>("_command_line") = std::move(command_line);

  std::string input_contents;
  input_contents += "Application/string_values='" + MooseUtils::stringJoin(string_values) + "'\n";
  input_contents += "Application/int_value=" + std::to_string(int_value) + "\n";

  auto parser = std::make_unique<Parser>("foo.i", std::optional<std::string>(input_contents));
  parser->parse();

  Moose::AppBuilder app_builder(std::move(parser));
  app_builder.buildParamsFromCommandLine("main", params, MPI_COMM_WORLD);

  EXPECT_EQ(params.get<std::vector<std::string>>("string_values"), string_values);
  EXPECT_EQ(params.get<bool>("bool_value"), bool_value);
  EXPECT_EQ(params.get<int>("int_value"), int_value);
  EXPECT_FALSE(params.isParamSetByUser("unsigned_int_value"));
}
