//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "Registry.h"

#include "Diffusion.h"
#include "MaterialRealAux.h"
#include "CheckOutputAction.h"
#include "MooseUtils.h"
#include "MooseUnitUtils.h"
#include "Capabilities.h"

#include <filesystem>

using Moose::internal::Capabilities;

class RegistryTest : public ::testing::Test
{
public:
  virtual void SetUp() override;
  virtual void TearDown() override;

  std::map<std::string, std::string> _old_data_file_paths;
  std::map<std::string, std::string> _old_repos;
  Moose::internal::Capabilities::RegistryType _old_capabilities_registry;
};

void
RegistryTest::SetUp()
{
  _old_data_file_paths = Registry::getDataFilePaths();
  Registry::setDataFilePaths({});

  _old_repos = Registry::getRepos();
  Registry::setRepos({});

  std::swap(Capabilities::getCapabilities({})._registry, _old_capabilities_registry);
}

void
RegistryTest::TearDown()
{
  Registry::setDataFilePaths(_old_data_file_paths);
  _old_data_file_paths.clear();

  Registry::setRepos(_old_repos);
  _old_repos.clear();

  std::swap(Capabilities::getCapabilities({})._registry, _old_capabilities_registry);
  _old_capabilities_registry.clear();
}

TEST_F(RegistryTest, getClassName)
{
  // This is a simple non-templated case
  EXPECT_EQ(Registry::getClassName<Diffusion>(), "Diffusion");

  // This is a templated case that would not work with demangle, as
  // demangle(typeid(ADMaterialRealAux).name()) returns
  // "MaterialRealAuxTempl<true>"
  EXPECT_EQ(Registry::getClassName<ADMaterialRealAux>(), "ADMaterialRealAux");

  // This tests the lookup of an action class name
  EXPECT_EQ(Registry::getClassName<CheckOutputAction>(), "CheckOutputAction");
}

TEST_F(RegistryTest, appNameFromAppPath)
{
  EXPECT_EQ(Registry::appNameFromAppPath("/path/to/FooBarBazApp.C"), "foo_bar_baz");
}

TEST_F(RegistryTest, appNameFromAppPathFailed)
{
  const std::string app_path = "/path/to/FooBarBazApp.h";
  EXPECT_MOOSEERROR_MSG(Registry::appNameFromAppPath(app_path),
                        "Registry::appNameFromAppPath(): Failed to parse application name from '" +
                            app_path + "'");
}

TEST_F(RegistryTest, addDataFilePath)
{
  // without info
  {
    const std::string name = "name";
    const std::string path = "files/data_file_tests/data0/data";
    Registry::addDataFilePath(name, path);

    // should add as a true capability
    const auto & capability = Capabilities::getCapabilities({}).get("data_" + name);
    EXPECT_TRUE(capability.getBoolValue());
    EXPECT_EQ(capability.getDoc(),
              "Named data path '" + name + "' is available at '" + MooseUtils::canonicalPath(path) +
                  "'.");
  }

  // with info
  {
    const std::string name = "name2";
    const std::string path = "files/data_file_tests/data0/data";
    const std::string info = "this is useful information";
    Registry::addDataFilePath(name, path, true, info);

    // should add as a true capability
    const auto & capability = Capabilities::getCapabilities({}).get("data_" + name);
    EXPECT_TRUE(capability.getBoolValue());
    EXPECT_EQ(capability.getDoc(),
              "Named data path '" + name + "' is available at '" + MooseUtils::canonicalPath(path) +
                  "'; " + info + ".");
  }
}

TEST_F(RegistryTest, addMissingDataFilePath)
{
  const std::string name = "name";
  const std::string info = "extra_info";
  Registry::addMissingDataFilePath(name, info);

  // should add as a false capability
  const auto & capability = Capabilities::getCapabilities({}).get("data_" + name);
  EXPECT_FALSE(capability.getBoolValue());
  EXPECT_EQ(capability.getDoc(), "Named data path '" + name + "' is not available; " + info + ".");
}

TEST_F(RegistryTest, addDataFilePathNonDataFolder)
{
  const std::string name = "non_data_folder";
  const std::string path = "files/data_file_tests/data0";
  const std::string folder = std::filesystem::path(path).filename().c_str();

  // Not allowed by default
  EXPECT_MOOSEERROR_MSG(Registry::addDataFilePath(name, path),
                        "While registering data file path '" + path + "' for '" + name +
                            "': The folder must be named 'data' and it is named '" + folder + "'");
  // Can be allowed if not an app
  Registry::addDataFilePath(name, path, false);
}

TEST_F(RegistryTest, addDataFilePathMismatch)
{
  const std::string name = "data_mismatch";
  const std::string path = "files/data_file_tests/data0/data";
  const std::string can_path = MooseUtils::canonicalPath(path);

  Registry::addDataFilePath(name, path);

  const std::string other_path = "files/data_file_tests/data1/data";
  const std::string other_can_path = MooseUtils::canonicalPath(other_path);

  EXPECT_MOOSEERROR_MSG(Registry::addDataFilePath(name, other_path),
                        "While registering data file path '" + other_can_path + "' for '" + name +
                            "': the path '" + can_path + "' is already registered");
}

TEST_F(RegistryTest, addDataFilePathBadName)
{
  EXPECT_MOOSEERROR_MSG(
      Registry::addDataFilePath("!", "unused"),
      "Unallowed characters in data file path name '!'; allowed characters = a-z, 0-9, _");
}

TEST_F(RegistryTest, addMissingDataFilePathBadName)
{
  EXPECT_MOOSEERROR_MSG(
      Registry::addMissingDataFilePath("!", "unused"),
      "Unallowed characters in data file path name '!'; allowed characters = a-z, 0-9, _");
}

TEST_F(RegistryTest, getDataPath)
{
  const std::string name = "data_working";
  const std::string path = "files/data_file_tests/data0/data";
  const std::string can_path = MooseUtils::canonicalPath(path);

  Registry::addDataFilePath(name, path);
  EXPECT_EQ(Registry::getDataFilePath(name), can_path);

  Registry::addDataFilePath(name, path);
  EXPECT_EQ(Registry::getDataFilePath(name), can_path);
}

TEST_F(RegistryTest, getDataPathUnregistered)
{
  const std::string name = "unregistered";
  EXPECT_THROW(
      {
        try
        {
          Registry::getDataFilePath(name);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Registry::getDataFilePath(): A data file path for '" + name +
                        "' is not registered");
          throw;
        }
      },
      std::exception);
}

TEST_F(RegistryTest, determineFilePath)
{
  const std::string path = "files/data_file_tests/data0/data";
  const std::string can_path = MooseUtils::canonicalPath(path);
  EXPECT_EQ(Registry::determineDataFilePath("unused", path), can_path);
}

TEST_F(RegistryTest, determineFilePathFailed)
{
  const std::string name = "unused";
  const std::string path = "foo";
  const std::string installed_path =
      MooseUtils::pathjoin(Moose::getExecutablePath(), "../share/" + name + "/data");

  EXPECT_MOOSEERROR_MSG(Registry::determineDataFilePath(name, path),
                        "Failed to determine data file path for '" + name +
                            "'. Paths searched:\n\n  installed: \"" + installed_path +
                            "\"\n  in-tree: " + path);
}

TEST_F(RegistryTest, addDeprecatedAppDataFilePath)
{
  const auto deprecated_is_error_before = Moose::_deprecated_is_error;
  Moose::_deprecated_is_error = false;

  Registry::addDeprecatedAppDataFilePath("../modules/solid_mechanics/src/base/SolidMechanicsApp.C");

  const std::string can_path = MooseUtils::canonicalPath("../modules/solid_mechanics/data");
  EXPECT_EQ(Registry::getDataFilePath("solid_mechanics"), can_path);

  Moose::_deprecated_is_error = deprecated_is_error_before;
}

TEST_F(RegistryTest, repositoryURL)
{
  const std::string repo_name = "bar";
  const std::string repo_url = "github.com/foo/bar";

  // not registered
  EXPECT_MOOSEERROR_MSG(Registry::getRepositoryURL(repo_name),
                        "Registry::getRepositoryURL(): The repository '" + repo_name +
                            "' is not registered.");

  // register it
  Registry::addRepository(repo_name, repo_url);
  EXPECT_EQ(Registry::getRepositoryURL(repo_name), repo_url);

  // re-register, different URL
  EXPECT_MOOSEERROR_MSG(Registry::addRepository(repo_name, "badurl"),
                        "Registry::registerRepository(): The repository '" + repo_name +
                            "' is already registered "
                            "with a different URL '" +
                            repo_url + "'.");
}
