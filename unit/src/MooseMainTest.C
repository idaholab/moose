//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseMain.h"

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
  EXPECT_EQ(app->parser().getAppType(), "MooseUnitApp");
}

TEST(MooseMainTest, createInputAppType)
{
  {
    Args args({"-i", "files/MooseMainTest/app_OtherMooseUnitApp.i"});
    const auto app = Moose::createMooseApp("unused", args.argc(), args.argv());
    EXPECT_EQ(app->parser().getAppType(), "OtherMooseUnitApp");
  }
  {
    Args args({"-i", "files/MooseMainTest/app_MooseUnitApp.i"});
    const auto app = Moose::createMooseApp("unused", args.argc(), args.argv());
    EXPECT_EQ(app->parser().getAppType(), "MooseUnitApp");
  }
}

TEST(MooseMainTest, createAppFromCLI)
{
  Args args({"--app", "OtherMooseUnitApp"});
  const auto app = Moose::createMooseApp("unused", args.argc(), args.argv());
  EXPECT_EQ(app->parser().getAppType(), "OtherMooseUnitApp");
}

TEST(MooseMainTest, createAppFromHITCLI)
{
  Args args({"Application/type=OtherMooseUnitApp"});
  const auto app = Moose::createMooseApp("unused", args.argc(), args.argv());
  EXPECT_EQ(app->parser().getAppType(), "OtherMooseUnitApp");
}

TEST(MooseMainTest, createLastCLIAppWins)
{
  Args args({"Application/type=BadApp", "--app=AnotherBadApp", "--app=OtherMooseUnitApp"});
  const auto app = Moose::createMooseApp("unused", args.argc(), args.argv());
  EXPECT_EQ(app->parser().getAppType(), "OtherMooseUnitApp");
}

TEST(MooseMainTest, createCLIWins)
{
  Args args({"-i", "files/MooseMainTest/app_MooseUnitApp.i", "--app", "OtherMooseUnitApp"});
  const auto app = Moose::createMooseApp("unused", args.argc(), args.argv());
  EXPECT_EQ(app->parser().getAppType(), "OtherMooseUnitApp");
}

TEST(MooseMainTest, createUnregistered)
{
  const std::string app_name = "unregistered";
  Args args({});
  EXPECT_THROW(
      {
        try
        {
          Moose::createMooseApp("unregistered", args.argc(), args.argv());
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "'" + app_name + "' is not a registered application type.");
          throw;
        }
      },
      std::exception);
}

TEST(MooseMainTest, createMultiAppApplicationOverrideError)
{
  const std::string arg = "subapp:Application/type=foo";
  Args args({arg});
  EXPECT_THROW(
      {
        try
        {
          Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "For command line argument '" + arg +
                        "': overriding the application type for MultiApps via command line is not "
                        "allowed.");
          throw;
        }
      },
      std::exception);
}
