//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseMain.h"
#include "Parser.h"
#include "MooseUnitUtils.h"

struct Args
{
  Args(const std::vector<std::string> & args) : _args(args)
  {
    _args.insert(_args.begin(), "/path/to/exe");
    for (auto & arg : _args)
      _argv.push_back((char *)arg.data());
    _argv.push_back(nullptr);
  }
  int argc() const { return _argv.size() - 1; }
  char ** argv() { return _argv.data(); }
  std::vector<std::string> _args;
  std::vector<char *> _argv;
};

TEST(MooseMainTest, createDefaultAppType)
{
  Args args({});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  EXPECT_TRUE(app->parser().getAppType().has_value());
  EXPECT_EQ(app->parser().getAppType()->first, "MooseUnitApp");
  EXPECT_EQ(app->parser().getAppType()->second, nullptr);
}

TEST(MooseMainTest, createInputAppType)
{
  {
    Args args({"-i", "files/MooseMainTest/app_OtherMooseUnitApp.i"});
    const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
    const auto type_node = app->parser().getRoot().find("Application/type");
    EXPECT_NE(type_node, nullptr);
    EXPECT_TRUE(app->parser().getAppType().has_value());
    EXPECT_EQ(app->parser().getAppType()->first, "OtherMooseUnitApp");
    EXPECT_EQ(app->parser().getAppType()->second, type_node);
  }
  {
    Args args({"-i", "files/MooseMainTest/app_MooseUnitApp.i"});
    const auto app = Moose::createMooseApp("OtherMooseUnitApp", args.argc(), args.argv());
    const auto type_node = app->parser().getRoot().find("Application/type");
    EXPECT_NE(type_node, nullptr);
    EXPECT_TRUE(app->parser().getAppType().has_value());
    EXPECT_EQ(app->parser().getAppType()->first, "MooseUnitApp");
    EXPECT_EQ(app->parser().getAppType()->second, type_node);
  }
}

TEST(MooseMainTest, createAppFromCLI)
{
  Args args({"--app", "OtherMooseUnitApp"});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  EXPECT_TRUE(app->parser().getAppType().has_value());
  EXPECT_EQ(app->parser().getAppType()->first, "OtherMooseUnitApp");
  EXPECT_EQ(app->parser().getAppType()->second, &app->parser().getCommandLineRoot());
}

TEST(MooseMainTest, createAppFromHITCLI)
{
  Args args({"Application/type=OtherMooseUnitApp"});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  EXPECT_TRUE(app->parser().getAppType().has_value());
  EXPECT_EQ(app->parser().getAppType()->first, "OtherMooseUnitApp");
  const auto node = app->parser().getCommandLineRoot().find("Application/type");
  EXPECT_NE(node, nullptr);
  EXPECT_EQ(app->parser().getAppType()->second, node);
}

TEST(MooseMainTest, createLastCLIAppWins)
{
  Args args({"Application/type=BadApp", "--app=AnotherBadApp", "--app=OtherMooseUnitApp"});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  EXPECT_TRUE(app->parser().getAppType().has_value());
  EXPECT_EQ(app->parser().getAppType()->first, "OtherMooseUnitApp");
}

TEST(MooseMainTest, createCLIWins)
{
  Args args({"-i", "files/MooseMainTest/app_MooseUnitApp.i", "--app", "OtherMooseUnitApp"});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  EXPECT_TRUE(app->parser().getAppType().has_value());
  EXPECT_EQ(app->parser().getAppType()->first, "OtherMooseUnitApp");
  EXPECT_EQ(app->parser().getAppType()->second, &app->parser().getCommandLineRoot());
}

TEST(MooseMainTest, createUnregistered)
{
  const std::string reg_name = "MooseUnitApp";
  const std::string unreg_name = "unregistered";

  // default app type is not registered
  {
    Args args({});
    Moose::UnitUtils::assertThrows<MooseRuntimeError>(
        [&args, &unreg_name]() { Moose::createMooseApp(unreg_name, args.argc(), args.argv()); },
        "createMooseApp: The default app type '" + unreg_name +
            "' is not a registered application type");
  }
  // not registered via --app
  {
    Args args({"--app=" + unreg_name});
    Moose::UnitUtils::assertThrows<MooseRuntimeError>(
        [&args, &reg_name]() { Moose::createMooseApp(reg_name, args.argc(), args.argv()); },
        "'" + unreg_name + "' is not a registered application type");
  }
  // not registered via Application/type
  {
    Args args({"Application/type=" + unreg_name});
    Moose::UnitUtils::assertThrows<MooseRuntimeError>(
        [&args, &reg_name]() { Moose::createMooseApp(reg_name, args.argc(), args.argv()); },
        Moose::hit_command_line_filename + ":Application/type:\n'" + unreg_name +
            "' is not a registered application type");
  }
}

TEST(MooseMainTest, createMultiAppApplicationOverrideError)
{
  const std::string arg = "subapp:Application/type=foo";
  Args args({arg});
  Moose::UnitUtils::assertThrows<MooseRuntimeError>(
      [&args]() { Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv()); },
      "For command line argument '" + arg +
          "': overriding the application type for MultiApps via command line is not allowed.");
}
