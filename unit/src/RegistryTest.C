//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "Registry.h"

#include "Diffusion.h"
#include "MaterialRealAux.h"
#include "CheckOutputAction.h"
#include "MooseUtils.h"

#include <filesystem>

TEST(RegistryTest, getClassName)
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

TEST(RegistryTest, appNameFromAppPath)
{
  EXPECT_EQ(Registry::appNameFromAppPath("/path/to/FooBarBazApp.C"), "foo_bar_baz");
}

TEST(RegistryTest, appNameFromAppPathFailed)
{
  const std::string app_path = "/path/to/FooBarBazApp.h";
  EXPECT_THROW(
      {
        try
        {
          Registry::appNameFromAppPath(app_path);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Registry::appNameFromAppPath(): Failed to parse application name from '" +
                        app_path + "'");
          throw;
        }
      },
      std::exception);
}

TEST(RegistryTest, addDataFilePathNonDataFolder)
{
  const std::string name = "non_data_folder";
  const std::string path = "foo";
  const std::string abs_path = MooseUtils::absolutePath(path);

  EXPECT_THROW(
      {
        try
        {
          Registry::addDataFilePath(name, path);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "While registering data file path '" + abs_path + "' for '" + name +
                        "': The folder must be named 'data' and it is named '" + path + "'");
          throw;
        }
      },
      std::exception);
}

TEST(RegistryTest, addDataFilePathMismatch)
{
  const std::string name = "data_mismatch";
  const std::string path = "data";
  const std::string abs_path = MooseUtils::absolutePath(path);

  Registry::addDataFilePath(name, path);

  const std::string other_path = "other_data/data";
  const std::string other_abs_path = MooseUtils::absolutePath(other_path);

  EXPECT_THROW(
      {
        try
        {
          Registry::addDataFilePath(name, other_path);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "While registering data file path '" + other_abs_path + "' for '" + name +
                        "': the path '" + abs_path + "' is already registered");
          throw;
        }
      },
      std::exception);
}

TEST(RegistryTest, getDataPath)
{
  const std::string name = "data_working";
  const std::string path = "data";
  const std::string abs_path = MooseUtils::absolutePath(path);

  Registry::addDataFilePath(name, path);
  EXPECT_EQ(Registry::getDataFilePath(name), abs_path);

  Registry::addDataFilePath(name, path);
  EXPECT_EQ(Registry::getDataFilePath(name), abs_path);
}

TEST(RegistryTest, getDataPathUnregistered)
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

TEST(RegistryTest, determineFilePath)
{
  const std::string path = "data";
  const std::string abs_path = MooseUtils::absolutePath(path);
  EXPECT_EQ(Registry::determineDataFilePath("unused", path), abs_path);
}

TEST(RegistryTest, determineFilePathFailed)
{
  const std::string name = "unused";
  const std::string path = "foo";
  const std::string abs_path = MooseUtils::absolutePath(path);
  const std::string installed_path = "../share/" + name + "/data";
  const std::string installed_abs_path = MooseUtils::absolutePath(installed_path);

  EXPECT_THROW(
      {
        try
        {
          Registry::determineDataFilePath(name, path);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Failed to determine data file path for '" + name +
                        "'. Paths searched:\n\n  installed: " + installed_abs_path +
                        "\n  in-tree: " + abs_path);
          throw;
        }
      },
      std::exception);
}

TEST(RegistryTest, repositoryURL)
{
  const std::string repo_name = "bar";
  const std::string repo_url = "github.com/foo/bar";

  // not registered
  EXPECT_THROW(
      {
        try
        {
          Registry::getRepositoryURL(repo_name);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Registry::getRepositoryURL(): The repository '" + repo_name +
                        "' is not registered.");
          throw;
        }
      },
      std::exception);

  // register it
  Registry::addRepository(repo_name, repo_url);
  EXPECT_EQ(Registry::getRepositoryURL(repo_name), repo_url);

  // re-register, different URL
  EXPECT_THROW(
      {
        try
        {
          Registry::addRepository(repo_name, "badurl");
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Registry::registerRepository(): The repository '" + repo_name +
                        "' is already registered "
                        "with a different URL '" +
                        repo_url + "'.");
          throw;
        }
      },
      std::exception);
}
