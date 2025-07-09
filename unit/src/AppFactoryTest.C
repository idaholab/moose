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
    try
    {
      af.getAppParamsID(params);
    }
    catch (const std::exception & e)
    {
      std::string msg(e.what());
      ASSERT_TRUE(msg.find("AppFactory::getAppParamsID(): Invalid application parameters (missing "
                           "'_app_params_id')") != std::string::npos)
          << msg;
    }
  }

  // Getting/clearing parameters that no longer exist
  {
    AppFactory af;
    InputParameters params = emptyInputParameters();
    af.storeAppParams(params);
    af.clearAppParams(params, {});

    try
    {
      af.getAppParamsID(params);
    }
    catch (const std::exception & e)
    {
      std::string msg(e.what());
      ASSERT_TRUE(
          msg.find("AppFactory::getAppParams(): Parameters for application with ID 0 not found") !=
          std::string::npos)
          << msg;
    }

    try
    {
      af.clearAppParams(params, {});
    }
    catch (const std::exception & e)
    {
      std::string msg(e.what());
      ASSERT_TRUE(
          msg.find(
              "AppFactory::clearAppParams(): Parameters for application with ID 0 not found") !=
          std::string::npos)
          << msg;
    }
  }
}

class CopyConstructParamsApp : public MooseApp
{
public:
  CopyConstructParamsApp(InputParameters parameters);

  static InputParameters validParams();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};

InputParameters
CopyConstructParamsApp::validParams()
{
  return MooseApp::validParams();
}

CopyConstructParamsApp::CopyConstructParamsApp(InputParameters parameters) : MooseApp(parameters)
{
  CopyConstructParamsApp::registerAll(_factory, _action_factory, _syntax);
}

void
CopyConstructParamsApp::registerAll(Factory & f, ActionFactory &, Syntax &)
{
  Registry::registerObjectsTo(f, {"CopyConstructParamsApp"});
}

TEST(AppFactoryTest, appCopyConstructParams)
{
  const std::string app_type = "CopyConstructParamsApp";

  auto & af = AppFactory::instance();
  af.reg<CopyConstructParamsApp>(app_type);
  auto params = af.getValidParams(app_type);

  auto command_line = std::make_shared<CommandLine>();
  command_line->addArguments({"exe", "--disable-perf-graph-live"});
  command_line->parse();
  params.set<std::shared_ptr<CommandLine>>("_command_line") = command_line;

  auto parser = std::make_shared<Parser>(std::vector<std::string>());
  parser->parse();
  params.set<std::shared_ptr<Parser>>("_parser") = parser;

  const auto deprecated_is_error = Moose::_deprecated_is_error;
  Moose::_deprecated_is_error = true;

  EXPECT_THROW(
      {
        try
        {
          af.create(app_type, "test", params, MPI_COMM_WORLD);
        }
        catch (const std::exception & e)
        {
          EXPECT_NE(std::string(e.what()).rfind(
                        "CopyConstructParamsApp copy-constructs its input parameters"),
                    std::string::npos)
              << e.what();
          throw;
        }
      },
      std::exception);

  Moose::_deprecated_is_error = deprecated_is_error;

  const auto it = af._name_to_build_info.find(app_type);
  EXPECT_NE(it, af._name_to_build_info.end());
  af._name_to_build_info.erase(it);

  EXPECT_EQ(af._input_parameters.size(), 1);
  af._input_parameters.clear();
}

TEST(AppFactoryTest, createNotRegistered)
{
  AppFactory af;

  try
  {
    af.create("fooapp", "unused", emptyInputParameters(), MPI_COMM_WORLD);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_TRUE(msg.find("AppFactory::Create(): Application 'fooapp' was not registered") !=
                std::string::npos)
        << msg;
  }
}

TEST(AppFactoryTest, createAppShared)
{
  const char * argv[2] = {"foo", "\0"};
  auto app = AppFactory::createAppShared("MooseUnitApp", 1, (char **)argv);
  ASSERT_NE(app, nullptr);
}
