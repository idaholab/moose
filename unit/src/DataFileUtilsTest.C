//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DataFileUtilsTest.h"

#include "Registry.h"
#include "DataFileUtils.h"
#include "MooseUtils.h"

#include <filesystem>

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
  Registry::setDataFilePaths({});
  Registry::addDataFilePath(_names[0], _paths[0]);
  Registry::addDataFilePath(_names[1], _paths[1]);
}

void
DataFileUtilsTest::TearDown()
{
  Registry::setDataFilePaths(_old_data_file_paths);
  _old_data_file_paths.clear();
}

void
DataFileUtilsTest::testData(const Moose::DataFileUtils::Path & path,
                            const unsigned int index,
                            const std::string & relative_path) const
{
  const auto can_path =
      MooseUtils::canonicalPath(MooseUtils::pathjoin(_can_paths[index], relative_path));
  EXPECT_EQ(path.path, can_path);
  EXPECT_EQ(path.context, Moose::DataFileUtils::Context::DATA);
  EXPECT_TRUE(path.data_name);
  EXPECT_EQ(*path.data_name, _names[index]);
}

void
DataFileUtilsTest::testRelative(const Moose::DataFileUtils::Path & path,
                                const std::string & relative_path) const
{
  const auto can_path = MooseUtils::canonicalPath(relative_path);
  EXPECT_EQ(path.path, can_path);
  EXPECT_EQ(path.context, Moose::DataFileUtils::Context::RELATIVE);
  EXPECT_FALSE(path.data_name);
}

void
DataFileUtilsTest::testAbsolute(const Moose::DataFileUtils::Path & path,
                                const std::string & can_path) const
{
  EXPECT_EQ(path.path, can_path);
  EXPECT_EQ(path.context, Moose::DataFileUtils::Context::ABSOLUTE);
  EXPECT_FALSE(path.data_name);
}

TEST_F(DataFileUtilsTest, getPathUnique)
{
  // Files that are unique to each data path
  testData(Moose::DataFileUtils::getPath("testdata0"), 0, "testdata0");
  testData(Moose::DataFileUtils::getPath("testdata1"), 1, "testdata1");
}

TEST_F(DataFileUtilsTest, getPathExplicit)
{
  testData(Moose::DataFileUtils::getPath(_names[0] + ":testdata"), 0, "testdata");
  testData(Moose::DataFileUtils::getPath(_names[1] + ":testdata"), 1, "testdata");
  testData(Moose::DataFileUtils::getPathExplicit(_names[0], "testdata"), 0, "testdata");
  testData(Moose::DataFileUtils::getPathExplicit(_names[1], "testdata"), 1, "testdata");
}

TEST_F(DataFileUtilsTest, getPathStartsWithDotSlash)
{
  const std::string path = "./testdata";

  // If the path starts with a dot, don't search the data directories. testdata
  // exists in both data directories.
  EXPECT_THROW(
      {
        try
        {
          Moose::DataFileUtils::getPath(path);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Unable to find the data file '" + path +
                        "' anywhere.\n\nData path(s) were not searched because search path begins "
                        "with './'.\n");
          throw;
        }
      },
      std::exception);

  // Path starts with a path, but base also contains testdata. data should
  // not be searched
  testRelative(Moose::DataFileUtils::getPath("./testdata", "files/data_file_tests"),
               "files/data_file_tests/testdata");
}

TEST_F(DataFileUtilsTest, getPathResolvesOutsideDot)
{
  // No base, path resolves outside . so nothing to search
  const std::string path = "../testdata";
  EXPECT_THROW(
      {
        try
        {
          Moose::DataFileUtils::getPath(path);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Unable to find the data file '" + path +
                        "' anywhere.\n\nData path(s) were not searched because search path "
                        "resolves behind '.'.\n");
          throw;
        }
      },
      std::exception);
}

TEST_F(DataFileUtilsTest, getPathMissing)
{
  const std::string file = "missingdata";
  const std::string cwd = std::filesystem::current_path().c_str();
  EXPECT_THROW(
      {
        try
        {
          Moose::DataFileUtils::getPath(file, cwd);
        }
        catch (const std::exception & e)
        {
          std::string err =
              "Unable to find the data file '" + file + "' anywhere.\n\nPaths searched:\n";
          auto paths = Registry::getRegistry().getDataFilePaths();
          paths.emplace("working directory", cwd);
          for (const auto & [name, data_path] : paths)
          {
            const std::string suffix = name == "working directory" ? "" : " data";
            err += "  " + name + suffix + ": " + data_path + "\n";
          }
          EXPECT_EQ(std::string(e.what()), err);
          throw;
        }
      },
      std::exception);
}

TEST_F(DataFileUtilsTest, getPathAmbiguous)
{
  const std::string file = "testdata";
  // testdata exists in both registered data
  EXPECT_THROW(
      {
        try
        {
          Moose::DataFileUtils::getPath(file);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Multiple files were found when searching for the data file 'testdata':\n\n  " +
                        _names[0] + ": " + _can_paths[0] + "/" + file + "\n  " + _names[1] + ": " +
                        _can_paths[1] + "/" + file +
                        "\n\nYou can resolve this ambiguity by appending a prefix with the desired "
                        "data name, for example:\n\n  " +
                        _names[0] + ":" + file);
          throw;
        }
      },
      std::exception);
}

TEST_F(DataFileUtilsTest, getPathRelative)
{
  const std::string relative_path = "files/data_file_tests/unregistered_data";

  // Not using registered data, relative path with base given
  testRelative(Moose::DataFileUtils::getPath(relative_path, _cwd), relative_path);

  // Not using registered data, cannot find without base
  EXPECT_THROW(
      {
        try
        {
          Moose::DataFileUtils::getPath(relative_path);
        }
        catch (const std::exception & e)
        {
          std::string err =
              "Unable to find the data file '" + relative_path + "' anywhere.\n\nPaths searched:\n";
          for (const auto & [name, data_path] : Registry::getRegistry().getDataFilePaths())
            err += "  " + name + " data: " + data_path + "\n";
          EXPECT_EQ(std::string(e.what()), err);
          throw;
        }
      },
      std::exception);
}

TEST_F(DataFileUtilsTest, getPathAbsolute)
{
  const auto can_path = MooseUtils::canonicalPath("files/data_file_tests/unregistered_data");

  // Not using registered data, absolute path. Should work with and without base
  testAbsolute(Moose::DataFileUtils::getPath(can_path), can_path);
  testAbsolute(Moose::DataFileUtils::getPath(can_path, _cwd), can_path);
}

TEST_F(DataFileUtilsTest, getPathAbsoluteExplicit)
{
  // Warning when specifying a data name with an absolute path
  EXPECT_THROW(
      {
        try
        {
          Moose::DataFileUtils::getPath(_names[0] + ":" + "/absolute/path");
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Should not specify an absolute path along with a data name to search "
                    "(requested to search in '" +
                        _names[0] + "')");
          throw;
        }
      },
      std::exception);
}

TEST_F(DataFileUtilsTest, getPathAbsoluteMissing)
{
  const auto can_path = MooseUtils::canonicalPath("files/data_file_tests/foo");

  // Absolute and just doesn't exist
  EXPECT_THROW(
      {
        try
        {
          testAbsolute(Moose::DataFileUtils::getPath(can_path), can_path);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "The absolute path '" + can_path + "' does not exist or is not readable.");
          throw;
        }
      },
      std::exception);
}

TEST_F(DataFileUtilsTest, getPathNameUnregistered)
{
  const std::string name = "unregistered_name";
  // Absolute and just doesn't exist
  EXPECT_THROW(
      {
        try
        {
          Moose::DataFileUtils::getPath(name + ":file");
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "Data from '" + name + "' is not registered to be searched");
          throw;
        }
      },
      std::exception);
}
