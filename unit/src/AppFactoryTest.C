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
#include "CommandLine.h"
#include "Parser.h"
#include "MooseUnitUtils.h"

TEST(AppFactoryTest, manageAppParams)
{
  {
    AppFactory af;

    // Store the parameters
    InputParameters params = emptyInputParameters();
    const auto & stored_params = af.storeAppParams(params);
    ASSERT_NE(&params, &stored_params);

    // Make sure IDs are consistent between the initial parameters
    // and the stored parameters
    const auto id = af.getAppParamsID(params);
    ASSERT_EQ(id, stored_params.get<std::size_t>("_app_params_id"));
    ASSERT_EQ(id, af.getAppParamsID(stored_params));

    // Make sure we point to the stored parameters
    ASSERT_EQ(&af.getAppParams(params), &stored_params);

    // Check clear
    af.clearAppParams(stored_params, {});
    ASSERT_EQ(af._input_parameters.count(id), 0);
  }

  // Parameters without an ID
  {
    AppFactory af;
    InputParameters params = emptyInputParameters();
    MOOSE_ASSERT_THROWS(
        MooseRuntimeError,
        af.getAppParamsID(params),
        "AppFactory::getAppParamsID(): Invalid application parameters (missing '_app_params_id')");
  }

  // Getting/clearing parameters that no longer exist
  {
    AppFactory af;
    InputParameters params = emptyInputParameters();
    af.storeAppParams(params);
    af.clearAppParams(params, {});
    MOOSE_ASSERT_THROWS(
        MooseRuntimeError,
        af.getAppParams(params),
        "AppFactory::getAppParams(): Parameters for application with ID 0 not found");
    MOOSE_ASSERT_THROWS(
        MooseRuntimeError,
        af.clearAppParams(params, {}),
        "AppFactory::clearAppParams(): Parameters for application with ID 0 not found");
  }
}

class CopyConstructParamsApp : public MooseApp
{
public:
  CopyConstructParamsApp(InputParameters parameters) : MooseApp(parameters) {}
  static InputParameters validParams() { return MooseApp::validParams(); }
  static void registerAll(Factory &, ActionFactory &, Syntax &) {}
};

TEST(AppFactoryTest, appCopyConstructParams)
{
  const std::string app_type = "CopyConstructParamsApp";

  auto & af = AppFactory::instance();
  af.reg<CopyConstructParamsApp>(app_type, "", 0);
  auto params = af.getValidParams(app_type);

  auto command_line = std::make_shared<CommandLine>();
  command_line->addArguments({"exe", "--disable-perf-graph-live"});
  command_line->parse();
  params.set<std::shared_ptr<CommandLine>>("_command_line") = command_line;

  auto parser = std::make_shared<Parser>(std::vector<std::string>());
  parser->parse();
  params.set<std::shared_ptr<Parser>>("_parser") = parser;

  ASSERT_EQ(af._input_parameters.size(), 0);

  const auto deprecated_is_error = Moose::_deprecated_is_error;
  Moose::_deprecated_is_error = true;
  MOOSE_ASSERT_THROWS(MooseRuntimeError,
                      af.create(app_type, "test", params, MPI_COMM_WORLD),
                      "CopyConstructParamsApp copy-constructs its input parameters");
  Moose::_deprecated_is_error = deprecated_is_error;

  ASSERT_EQ(af._input_parameters.size(), 1);

  const auto it = af._name_to_build_info.find(app_type);
  EXPECT_NE(it, af._name_to_build_info.end());
  af._name_to_build_info.erase(it);

  af._input_parameters.clear();
  af._next_input_parameters_id = 0;
}

TEST(AppFactoryTest, createNotRegistered)
{
  AppFactory af;
  MOOSE_ASSERT_THROWS(MooseRuntimeError,
                      af.create("fooapp", "unused", emptyInputParameters(), MPI_COMM_WORLD),
                      "AppFactory::Create(): Application 'fooapp' was not registered");
}

TEST(AppFactoryTest, createForUnit)
{
  {
    auto app = AppFactory::create("MooseUnitApp");
    EXPECT_NE(app, nullptr);
    auto & args = app->commandLine()->getArguments();
    EXPECT_EQ(args.size(), 1);
    EXPECT_EQ(args[0], "unused");
  }

  {
    auto app = AppFactory::create("MooseUnitApp", {"--help"});
    EXPECT_NE(app, nullptr);
    auto & args = app->commandLine()->getArguments();
    EXPECT_EQ(args.size(), 2);
    EXPECT_EQ(args[0], "unused");
    EXPECT_EQ(args[1], "--help");
  }
}

TEST(AppFactoryTest, createAppShared)
{
  const char * argv[2] = {"foo", "\0"};
  auto app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  ASSERT_NE(app, nullptr);
}
