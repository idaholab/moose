//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Console.h"
#include "Moose.h"
#include "MooseApp.h"
#include "MooseMain.h"

namespace
{
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
}

// Verify that setting execute_on on a Console propagates to sub-type flags
// (postprocessors, scalars, etc.) that the user did not explicitly configure.
TEST(ConsoleExecuteOnTest, executeOnFinalPropagates)
{
  Args args({"-i", "files/ConsoleExecuteOnTest/execute_on_final.i"});
  auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();

  Console * console = app->getOutputWarehouse().getOutput<Console>("console");
  ASSERT_NE(console, nullptr);

  const auto & exec_on = console->advancedExecuteOn();

  static const std::array<std::string, 4> execute_types = {
      "postprocessors", "scalars", "vector_postprocessors", "reporters"};
  for (const std::string & key : execute_types)
  {
    auto it = exec_on.find(key);
    ASSERT_NE(it, exec_on.end()) << "missing key: " << key;
    EXPECT_TRUE(it->second.isValueSet(EXEC_FINAL)) << "key: " << key;
    EXPECT_FALSE(it->second.isValueSet(EXEC_INITIAL)) << "key: " << key;
    EXPECT_FALSE(it->second.isValueSet(EXEC_TIMESTEP_END)) << "key: " << key;
  }
}

// Verify that an explicitly set sub-type flag is not overridden by execute_on.
TEST(ConsoleExecuteOnTest, explicitSubTypeFlagIsRespected)
{
  Args args({"-i", "files/ConsoleExecuteOnTest/execute_on_with_subtype.i"});
  auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();

  Console * console = app->getOutputWarehouse().getOutput<Console>("console");
  ASSERT_NE(console, nullptr);

  const auto & exec_on = console->advancedExecuteOn();

  // postprocessors was explicitly set to timestep_end - execute_on must not override it
  {
    auto it = exec_on.find("postprocessors");
    ASSERT_NE(it, exec_on.end());
    EXPECT_TRUE(it->second.isValueSet(EXEC_TIMESTEP_END));
    EXPECT_FALSE(it->second.isValueSet(EXEC_FINAL));
  }

  // scalars was not explicitly set - it should inherit execute_on = final
  {
    auto it = exec_on.find("scalars");
    ASSERT_NE(it, exec_on.end());
    EXPECT_TRUE(it->second.isValueSet(EXEC_FINAL));
    EXPECT_FALSE(it->second.isValueSet(EXEC_INITIAL));
    EXPECT_FALSE(it->second.isValueSet(EXEC_TIMESTEP_END));
  }
}
