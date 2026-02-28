//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest_include.h"

#include "Registry.h"
#include "DataFileUtils.h"
#include "MooseUtils.h"
#include "MooseUnitUtils.h"
#include "Capabilities.h"

#include <filesystem>

using Moose::DataFileUtils::Context;
using Moose::DataFileUtils::getPath;
using Moose::DataFileUtils::getPathExplicit;
using Moose::DataFileUtils::GetPathOptions;
using Moose::DataFileUtils::Path;
using Moose::internal::Capabilities;

class DataFileUtilsTest : public ::testing::Test
{
public:
  DataFileUtilsTest();

  virtual void SetUp() override;
  virtual void TearDown() override;

  void
  testData(const Path & path, const unsigned int index, const std::string & relative_path) const;
  void testRelative(const Path & path, const std::string & relative_path) const;
  void testAbsolute(const Path & path, const std::string & absolute_path) const;

  const std::array<std::string, 2> _names;
  const std::array<std::string, 2> _paths;
  const std::array<std::string, 2> _can_paths;
  const std::string _cwd;
  std::map<std::string, std::string> _old_data_file_paths;
  Capabilities::RegistryType _old_capabilities_registry;
};

DataFileUtilsTest::DataFileUtilsTest()
  : ::testing::Test(),
    _names({"data0", "data1"}),
    _paths({"files/data_file_tests/data0/data", "files/data_file_tests/data1/data"}),
    _can_paths({MooseUtils::canonicalPath(_paths[0]), MooseUtils::canonicalPath(_paths[1])}),
    _cwd(std::filesystem::current_path().c_str())
{
}

void
DataFileUtilsTest::SetUp()
{
  _old_data_file_paths = Registry::getDataFilePaths();
  std::swap(Capabilities::getCapabilities({})._registry, _old_capabilities_registry);
  Registry::setDataFilePaths({});
  Registry::addDataFilePath(_names[0], _paths[0]);
  Registry::addDataFilePath(_names[1], _paths[1]);
}

void
DataFileUtilsTest::TearDown()
{
  Registry::setDataFilePaths(_old_data_file_paths);
  _old_data_file_paths.clear();
  std::swap(Capabilities::getCapabilities({})._registry, _old_capabilities_registry);
  _old_capabilities_registry.clear();
}

void
DataFileUtilsTest::testData(const Path & path,
                            const unsigned int index,
                            const std::string & relative_path) const
{
  const auto can_path =
      MooseUtils::canonicalPath(MooseUtils::pathjoin(_can_paths[index], relative_path));
  EXPECT_EQ(path.path, can_path);
  EXPECT_EQ(path.context, Context::DATA);
  EXPECT_TRUE(path.data_name);
  EXPECT_EQ(*path.data_name, _names[index]);
}

void
DataFileUtilsTest::testRelative(const Path & path, const std::string & relative_path) const
{
  const auto can_path = MooseUtils::canonicalPath(relative_path);
  EXPECT_EQ(path.path, can_path);
  EXPECT_EQ(path.context, Context::RELATIVE);
  EXPECT_FALSE(path.data_name);
}

void
DataFileUtilsTest::testAbsolute(const Path & path, const std::string & can_path) const
{
  EXPECT_EQ(path.path, can_path);
  EXPECT_EQ(path.context, Context::ABSOLUTE);
  EXPECT_FALSE(path.data_name);
}

TEST_F(DataFileUtilsTest, getPathUnique)
{
  // Files that are unique to each data path
  testData(getPath("testdata0"), 0, "testdata0");
  testData(getPath("testdata1"), 1, "testdata1");
}

TEST_F(DataFileUtilsTest, getPathExplicit)
{
  testData(getPath(_names[0] + ":testdata"), 0, "testdata");
  testData(getPath(_names[1] + ":testdata"), 1, "testdata");
  testData(getPathExplicit(_names[0], "testdata"), 0, "testdata");
  testData(getPathExplicit(_names[1], "testdata"), 1, "testdata");
}

TEST_F(DataFileUtilsTest, getPathStartsWithDotSlash)
{
  const std::string path = "./testdata";

  // If the path starts with a dot, don't search the data directories. testdata
  // exists in both data directories.
  EXPECT_MOOSEERROR_MSG(
      getPath(path),
      "Unable to find the data file '" + path +
          "'.\n\nPaths searched:\n  working directory: " + std::filesystem::current_path().c_str() +
          "\n\nData path(s) were not searched because search path begins with './'.\n");

  // Path starts with a path, but base also contains testdata. data should
  // not be searched
  {
    GetPathOptions options;
    options.base = "files/data_file_tests";
    testRelative(getPath("./testdata", options), "files/data_file_tests/testdata");
  }
}

TEST_F(DataFileUtilsTest, getPathResolvesOutsideDot)
{
  // No base, path resolves outside . so nothing to search
  const std::string path = "../testdata";
  EXPECT_MOOSEERROR_MSG(
      getPath(path),
      "Unable to find the data file '" + path +
          "'.\n\nPaths searched:\n  working directory: " + std::filesystem::current_path().c_str() +
          "\n\nData path(s) were not searched because search path resolves behind '.'.\n");
}

TEST_F(DataFileUtilsTest, getPathMissing)
{
  const std::string file = "missingdata";
  const std::string cwd = std::filesystem::current_path().c_str();
  std::string err = "Unable to find the data file '" + file + "'.\n\nPaths searched:\n";
  auto paths = Registry::getRegistry().getDataFilePaths();
  paths.emplace("working directory", cwd);
  for (const auto & [name, data_path] : paths)
  {
    const std::string suffix = name == "working directory" ? "" : " data";
    err += "  " + name + suffix + ": " + data_path + "\n";
  }
  GetPathOptions options;
  options.base = cwd;
  EXPECT_MOOSEERROR_MSG(getPath(file, options), err);
}

TEST_F(DataFileUtilsTest, getPathAmbiguous)
{
  // Test getting a data file path is ambiguous
  const std::string file = "testdata";
  EXPECT_MOOSEERROR_MSG(
      getPath(file),
      "Multiple files were found when searching for the data file 'testdata':\n\n  " + _names[0] +
          ": " + _can_paths[0] + "/" + file + "\n  " + _names[1] + ": " + _can_paths[1] + "/" +
          file +
          "\n\nYou can resolve this ambiguity by appending a prefix with the desired "
          "data name, for example:\n\n  " +
          _names[0] + ":" + file);
}

TEST_F(DataFileUtilsTest, getPathRelative)
{
  // Not using registered data, relative path with base given
  const std::string relative_path = "files/data_file_tests/unregistered_data";
  GetPathOptions options;
  options.base = _cwd;
  testRelative(getPath(relative_path, options), relative_path);
}

TEST_F(DataFileUtilsTest, getPathAbsolute)
{
  const auto can_path = MooseUtils::canonicalPath("files/data_file_tests/unregistered_data");

  // Not using registered data, absolute path. Should work with and without base
  testAbsolute(getPath(can_path), can_path);
  {
    GetPathOptions options;
    options.base = _cwd;
    testAbsolute(getPath(can_path, options), can_path);
  }
}

TEST_F(DataFileUtilsTest, getPathAbsoluteGraceful)
{
  // Absolute path that doesn't exist but with graceful and no data search
  // should return a not found path but still a path
  const std::string bad_path = "/no/exist";
  GetPathOptions options;
  options.graceful = true;
  options.search_all_data = false;
  const auto path = getPath(bad_path, options);
  EXPECT_EQ(path.path, bad_path);
  EXPECT_EQ(path.context, Context::ABSOLUTE_NOT_FOUND);
  EXPECT_FALSE(path.data_name);
}

TEST_F(DataFileUtilsTest, getPathAbsoluteExplicit)
{
  // Error when specifying a data name with an absolute path
  EXPECT_MOOSEERROR_MSG(getPath(_names[0] + ":" + "/absolute/path"),
                        "Cannot use an absolute path along with a data name to search "
                        "(requested to search in '" +
                            _names[0] + "')");
}

TEST_F(DataFileUtilsTest, getPathRelativeExplicit)
{
  // Error when specifying a data name with a relative path
  EXPECT_MOOSEERROR_MSG(getPath(_names[0] + ":" + "./absolute/path"),
                        "Cannot use a path that starts with './' along with a data name to search "
                        "(requested to search in '" +
                            _names[0] + "')");
  EXPECT_MOOSEERROR_MSG(
      getPath(_names[0] + ":" + "../behind"),
      "Cannot use a relative path along with a data name to search (requested to search in '" +
          _names[0] + "')");
}

TEST_F(DataFileUtilsTest, getPathRelativeGraceful)
{
  // Relative path that doesn't exist but with graceful and no data search
  // should return a not found path but still a path
  const std::string bad_path = "no_exist";
  GetPathOptions options;
  options.graceful = true;
  options.search_all_data = false;
  const auto path = getPath(bad_path, options);
  EXPECT_EQ(path.path, MooseUtils::pathjoin(std::filesystem::current_path(), bad_path));
  EXPECT_EQ(path.context, Context::RELATIVE_NOT_FOUND);
  EXPECT_FALSE(path.data_name);
}

TEST_F(DataFileUtilsTest, getPathExplicitNotFound)
{
  // With an explicit data name, if the file isn't found we should always error
  EXPECT_MOOSEERROR_MSG(getPath(_names[0] + ":no_exist"),
                        "The path 'no_exist' was not found in data from '" + _names[0] + "'");
}

TEST_F(DataFileUtilsTest, getPathAbsoluteMissing)
{
  const auto can_path = MooseUtils::canonicalPath("files/data_file_tests/foo");

  // Absolute and just doesn't exist
  EXPECT_MOOSEERROR_MSG(testAbsolute(getPath(can_path), can_path),
                        "The absolute path '" + can_path + "' does not exist or is not readable.");
}

TEST_F(DataFileUtilsTest, getPathNameUnregistered)
{
  const std::string name = "unregistered_name";
  // Absolute and just doesn't exist
  EXPECT_MOOSEERROR_MSG(getPath(name + ":file"),
                        "Data from '" + name + "' is not registered to be searched");
}
