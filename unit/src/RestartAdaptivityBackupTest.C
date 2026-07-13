//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseMain.h"
#include "MultiApp.h"
#include "NonlinearSystem.h"

#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace
{
struct Args
{
  Args(const std::vector<std::string> & args) : _args(args)
  {
    _args.insert(_args.begin(), "unused");
    for (auto & arg : _args)
      _argv.push_back(arg.data());
    _argv.push_back(nullptr);
  }
  int argc() const { return static_cast<int>(_argv.size()) - 1; }
  char ** argv() { return _argv.data(); }
  std::vector<std::string> _args;
  std::vector<char *> _argv;
};

struct ScopedTempDir
{
  ScopedTempDir()
    : path(std::filesystem::temp_directory_path() /
           ("moose_restart_adaptivity_unit_" +
            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count())))
  {
    std::filesystem::remove_all(path);
    std::filesystem::create_directories(path);
  }

  ~ScopedTempDir() { std::filesystem::remove_all(path); }

  std::filesystem::path path;
};

MultiApp &
singleMultiApp(FEProblemBase & problem)
{
  const auto & multi_apps = problem.getMultiAppWarehouse().getObjects();
  mooseAssert(multi_apps.size() == 1, "Expected one MultiApp");
  return *multi_apps.front();
}
}

TEST(RestartAdaptivityBackupTest, restoresAdaptedSubAppMeshAndSystemData)
{
  ScopedTempDir root;

  const std::string parent_input = "files/RestartAdaptivityBackupTest/parent.i";
  const std::string parent_restart_input = "files/RestartAdaptivityBackupTest/parent_restart.i";
  const std::string first_file_base = (root.path / "parent").string();
  const std::string restart_file_base = first_file_base + "_cp/0001";
  const std::string second_file_base = (root.path / "parent_restart").string();

  {
    Args args({"-i", parent_input, "Outputs/file_base=" + first_file_base});
    const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
    app->run();
  }

  Args args({"-i",
             parent_restart_input,
             "Problem/restart_file_base=" + restart_file_base,
             "Outputs/file_base=" + second_file_base});
  const auto app = Moose::createMooseApp("MooseUnitApp", args.argc(), args.argv());
  app->run();

  auto & multi_app = singleMultiApp(app->getExecutioner()->feProblem());
  ASSERT_TRUE(multi_app.hasApp());
  ASSERT_EQ(multi_app.numLocalApps(), 1);

  MooseApp & sub_app = *multi_app.localApp(0);
  EXPECT_TRUE(sub_app.restoredInitialBackupMesh());

  auto & sub_problem = sub_app.getExecutioner()->feProblem();
  const auto & mesh = sub_problem.mesh().getMesh();
  EXPECT_EQ(mesh.n_active_elem(), 16);
  EXPECT_EQ(mesh.n_nodes(), 25);
  EXPECT_EQ(sub_problem.getNonlinearSystem(0).system().n_dofs(), 25);
}
