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
    _names({"DataFileUtilsTest_data0", "DataFileUtilsTest_data1"}),
    _paths({"files/data_file_tests/data0/data", "files/data_file_tests/data1/data"}),
    _abs_paths({MooseUtils::absolutePath(_paths[0]), MooseUtils::absolutePath(_paths[1])}),
    _cwd(std::filesystem::current_path().c_str())
{
  Registry::addDataFilePath(_names[0], _paths[0]);
  Registry::addDataFilePath(_names[1], _paths[1]);
}

void
DataFileUtilsTest::testData(const Moose::DataFileUtils::Path & path,
                            const unsigned int index,
                            const std::string & relative_path) const
{
  const auto abs_path =
      MooseUtils::absolutePath(MooseUtils::pathjoin(_abs_paths[index], relative_path));
  EXPECT_EQ(path.path, abs_path);
  EXPECT_EQ(path.context, Moose::DataFileUtils::Context::DATA);
  EXPECT_TRUE(path.data_name);
  EXPECT_EQ(*path.data_name, _names[index]);
}

void
DataFileUtilsTest::testRelative(const Moose::DataFileUtils::Path & path,
                                const std::string & relative_path) const
{
  const auto abs_path = MooseUtils::absolutePath(relative_path);
  EXPECT_EQ(path.path, abs_path);
  EXPECT_EQ(path.context, Moose::DataFileUtils::Context::RELATIVE);
  EXPECT_FALSE(path.data_name);
}

void
DataFileUtilsTest::testAbsolute(const Moose::DataFileUtils::Path & path,
                                const std::string & absolute_path) const
{
  EXPECT_EQ(path.path, absolute_path);
  EXPECT_EQ(path.context, Moose::DataFileUtils::Context::ABSOLUTE);
  EXPECT_FALSE(path.data_name);
}

TEST_F(DataFileUtilsTest, getDataUnique)
{
  // Files that are unique to each data path
  testData(Moose::DataFileUtils::getPath("testdata0"), 0, "testdata0");
  testData(Moose::DataFileUtils::getPath("testdata1"), 1, "testdata1");
}

TEST_F(DataFileUtilsTest, getDataNotUnique)
{

  testData(Moose::DataFileUtils::getPath("testdata", {}, _names[0]), 0, "testdata");
  testData(Moose::DataFileUtils::getPath("testdata", {}, _names[1]), 1, "testdata");
}

TEST_F(DataFileUtilsTest, getDataAmbiguous)
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
                        _names[0] + ": " + _abs_paths[0] + "/" + file + "\n  " + _names[1] + ": " +
                        _abs_paths[1] + "/" + file + "\n");
          throw;
        }
      },
      std::exception);
}

TEST_F(DataFileUtilsTest, getDataRelative)
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
            err += "  " + name + ": " + data_path + "\n";
          EXPECT_EQ(std::string(e.what()), err);
          throw;
        }
      },
      std::exception);
}

TEST_F(DataFileUtilsTest, getDataAbsolute)
{
  const auto absolute_path = MooseUtils::absolutePath("files/data_file_tests/unregistered_data");

  // Not using registered data, absolute path. Should work with and without base
  testAbsolute(Moose::DataFileUtils::getPath(absolute_path), absolute_path);
  testAbsolute(Moose::DataFileUtils::getPath(absolute_path, _cwd), absolute_path);
}

TEST_F(DataFileUtilsTest, getDataAbsoluteMissing)
{
  const auto absolute_path = MooseUtils::absolutePath("files/data_file_tests/foo");

  // Absolute and just doesn't exist
  EXPECT_THROW(
      {
        try
        {
          testAbsolute(Moose::DataFileUtils::getPath(absolute_path), absolute_path);
        }
        catch (const std::exception & e)
        {
          EXPECT_EQ(std::string(e.what()),
                    "The absolute path '" + absolute_path + "' does not exist or is not readable.");
          throw;
        }
      },
      std::exception);
}
