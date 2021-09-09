//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

// MOOSE includes
#include "DelimitedFileReader.h"
#include "MooseException.h"
#include "libmesh/point.h"

TEST(DelimitedFileReader, BadFilename)
{
  try
  {
    MooseUtils::DelimitedFileReader reader("not/a/valid/file.csv");
    reader.read();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("Unable to open file \"not/a/valid/file.csv\"");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(DelimitedFileReader, BadHeaderName)
{
  try
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.read();
    reader.getData("second");
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what())
                          .find("Could not find 'second' in header of file "
                                "data/csv/example.csv.");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(DelimitedFileReader, BadIndex)
{
  try
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.read();
    reader.getData(12345);
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::size_t pos = std::string(err.what()).find("The supplied index 12345 is out-of-range");
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(DelimitedFileReader, Headers)
{
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.read();
    const std::vector<std::string> & cols = reader.getNames();
    std::vector<std::string> gold = {"year", "month", "day"};
    EXPECT_EQ(cols, gold);
  }
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_no_header.csv");
    reader.read();
    const std::vector<std::string> & cols = reader.getNames();
    std::vector<std::string> gold = {"column_0", "column_1", "column_2"};
    EXPECT_EQ(cols, gold);
  }
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_padding_test.csv");
    reader.read();
    const std::vector<std::string> & cols = reader.getNames();
    std::vector<std::string> gold = {"column_00",
                                     "column_01",
                                     "column_02",
                                     "column_03",
                                     "column_04",
                                     "column_05",
                                     "column_06",
                                     "column_07",
                                     "column_08",
                                     "column_09",
                                     "column_10",
                                     "column_11",
                                     "column_12",
                                     "column_13"};
    EXPECT_EQ(cols, gold);
  }
}

TEST(DelimitedFileReader, Data)
{
  std::vector<std::vector<double>> gold = {{1980, 1980, 2011, 2013}, {6, 10, 5, 5}, {24, 9, 1, 15}};

  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.read();
    const std::vector<std::vector<double>> & data = reader.getData();
    EXPECT_EQ(data, gold);
  }
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.read();
    EXPECT_EQ(reader.getData("day"), gold[2]);
    EXPECT_EQ(reader.getData(2), gold[2]);
  }
}

TEST(DelimitedFileReader, DataChangeDelimiter)
{
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.txt");
    reader.setDelimiter("$");
    reader.read();
    const std::vector<std::vector<double>> & data = reader.getData();
    std::vector<std::vector<double>> gold = {
        {1980, 1980, 2011, 2013}, {6, 10, 5, 5}, {24, 9, 1, 15}};
    EXPECT_EQ(data, gold);
  }
}

TEST(DelimitedFileReader, WrongPointSize)
{
  try
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_bad_points.csv");
    reader.read();
    const std::vector<Point> pts = reader.getDataAsPoints();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::string gold = "Each point in file data/csv/example_bad_points.csv must have 3 entries";
    std::size_t pos = std::string(err.what()).find(gold);
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(DelimitedFileReader, WrongRowSize)
{
  try
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_bad_row.csv");
    reader.read();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::string gold = "The number of columns read (4) does not match the number of columns "
                       "expected (3) based on the first row of the file when reading row 4 in file "
                       "data/csv/example_bad_row.csv.";
    std::size_t pos = std::string(err.what()).find(gold);
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(DelimitedFileReader, BadRowValue)
{
  try
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_bad_value.csv");
    reader.read();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::string gold = "Failed to convert a delimited data into double when reading line 4 in file "
                       "data/csv/example_bad_value.csv.";
    std::size_t pos = std::string(err.what()).find(gold);
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(DelimitedFileReader, EmptyLine)
{
  // Read file with empty lines
  MooseUtils::DelimitedFileReader reader("data/csv/example_empty_lines.csv");
  reader.read();

  // Disable empty lines
  try
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_empty_lines.csv");
    reader.setIgnoreEmptyLines(false);
    reader.read();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::string gold =
        "Failed to read line 4 in file data/csv/example_empty_lines.csv. The line is empty.";
    std::size_t pos = std::string(err.what()).find(gold);
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(DelimitedFileReader, RowData)
{
  // Read file with empty lines
  MooseUtils::DelimitedFileReader reader("data/csv/row_example.csv");
  reader.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
  reader.read();

  std::vector<std::vector<double>> gold = {{1, 2, 3, 4}, {10, 20, 30, 40, 50}};
  std::vector<std::string> names = {"foo", "bar"};
  EXPECT_EQ(reader.getData(), gold);
  EXPECT_EQ(reader.getData("foo"), gold[0]);
  EXPECT_EQ(reader.getData("bar"), gold[1]);
  EXPECT_EQ(reader.getNames(), names);
}

TEST(DelimitedFileReader, RowDataNoHeader)
{
  // Read file with empty lines
  MooseUtils::DelimitedFileReader reader("data/csv/row_example_no_header.csv");
  reader.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
  reader.read();

  std::vector<std::vector<double>> gold = {{1, 2, 3, 4},
                                           {10, 20, 30, 40, 50},
                                           {300, 301},
                                           {300, 301},
                                           {300, 301},
                                           {300, 301},
                                           {300, 301},
                                           {300, 301},
                                           {300, 301},
                                           {300, 301},
                                           {300, 301}};
  std::vector<std::string> names = {"row_00",
                                    "row_01",
                                    "row_02",
                                    "row_03",
                                    "row_04",
                                    "row_05",
                                    "row_06",
                                    "row_07",
                                    "row_08",
                                    "row_09",
                                    "row_10"};
  EXPECT_EQ(reader.getData(), gold);
  EXPECT_EQ(reader.getNames(), names);
  EXPECT_EQ(reader.getData("row_00"), gold[0]);
  EXPECT_EQ(reader.getData("row_01"), gold[1]);
  EXPECT_EQ(reader.getData("row_02"), gold[2]);
}

TEST(DelimitedFileReader, RowDataComment)
{
  MooseUtils::DelimitedFileReader reader("data/csv/row_example_comment.csv");
  reader.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
  reader.setComment("#");
  reader.read();

  std::vector<std::vector<double>> gold = {
      {0, 1, 2, 3}, {1, 4, 2, 1, 113, 31}, {2, 4, 5, 6}, {3, 5, 4, 3, 2, 1}};
  EXPECT_EQ(reader.getData(), gold);
  EXPECT_EQ(reader.getData("row_0"), gold[0]);
  EXPECT_EQ(reader.getData("row_1"), gold[1]);
  EXPECT_EQ(reader.getData("row_2"), gold[2]);
  EXPECT_EQ(reader.getData("row_3"), gold[3]);
}

TEST(DelimitedFileReader, Comment)
{
  MooseUtils::DelimitedFileReader reader("data/csv/row_example_line_comment.csv");
  reader.setDelimiter(",");
  reader.setComment("#");
  reader.read();

  std::vector<std::vector<double>> gold = {{0, 0}, {1, 4}, {2, 5}, {3, 6}};
  EXPECT_EQ(reader.getData(), gold);
  EXPECT_EQ(reader.getData("column_0"), gold[0]);
  EXPECT_EQ(reader.getData("column_1"), gold[1]);
  EXPECT_EQ(reader.getData("column_2"), gold[2]);
  EXPECT_EQ(reader.getData("column_3"), gold[3]);
}

TEST(DelimitedFileReader, AutoDelimiter)
{
  MooseUtils::DelimitedFileReader reader("data/csv/example_space.csv");
  reader.read();

  std::vector<std::vector<double>> gold = {{1980, 1980, 2011, 2013}, {6, 10, 5, 5}, {24, 9, 1, 15}};
  EXPECT_EQ(reader.getData(), gold);
  EXPECT_EQ(reader.getData("year"), gold[0]);
  EXPECT_EQ(reader.getData("month"), gold[1]);
  EXPECT_EQ(reader.getData("day"), gold[2]);
}

TEST(DelimitedFileReader, AutoDelimiterComment)
{
  MooseUtils::DelimitedFileReader reader("data/csv/example_space_comment.csv");
  reader.setFormatFlag(MooseUtils::DelimitedFileReader::FormatFlag::ROWS);
  reader.setComment("#");
  reader.read();

  std::vector<std::vector<double>> gold = {{0, 1, 2, 3}, {0, 4, 5, 6}};
  EXPECT_EQ(reader.getData(), gold);
  EXPECT_EQ(reader.getData("row_0"), gold[0]);
  EXPECT_EQ(reader.getData("row_1"), gold[1]);
}

TEST(DelimitedFileReader, AutoHeader)
{
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    // Set the header flag to "AUTO", which is the default, but this is what is being tested
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::AUTO);
    reader.read();

    std::vector<std::vector<double>> gold = {
        {1980, 1980, 2011, 2013}, {6, 10, 5, 5}, {24, 9, 1, 15}};
    EXPECT_EQ(reader.getData(), gold);
    EXPECT_EQ(reader.getData("year"), gold[0]);
    EXPECT_EQ(reader.getData("month"), gold[1]);
    EXPECT_EQ(reader.getData("day"), gold[2]);
  }

  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::ON);
    reader.read();

    std::vector<std::vector<double>> gold = {
        {1980, 1980, 2011, 2013}, {6, 10, 5, 5}, {24, 9, 1, 15}};
    EXPECT_EQ(reader.getData(), gold);
    EXPECT_EQ(reader.getData("year"), gold[0]);
    EXPECT_EQ(reader.getData("month"), gold[1]);
    EXPECT_EQ(reader.getData("day"), gold[2]);
  }

  try
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::OFF);
    reader.read();
    FAIL();
  }
  catch (const std::exception & err)
  {
    std::string gold = "Failed to convert a delimited data into double when reading line 1";
    std::size_t pos = std::string(err.what()).find(gold);
    ASSERT_TRUE(pos != std::string::npos);
  }
}

TEST(DelimitedFileReader, AutoNoHeader)
{
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_no_header.csv");
    // Set the header flag to "AUTO", which is the default, but this is what is being tested
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::AUTO);
    reader.read();

    std::vector<std::vector<double>> gold = {
        {1980, 1980, 2011, 2013}, {6, 10, 5, 5}, {24, 9, 1, 15}};
    EXPECT_EQ(reader.getData(), gold);
    EXPECT_EQ(reader.getData("column_0"), gold[0]);
    EXPECT_EQ(reader.getData("column_1"), gold[1]);
    EXPECT_EQ(reader.getData("column_2"), gold[2]);
  }

  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_no_header.csv");
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::OFF);
    reader.read();

    std::vector<std::vector<double>> gold = {
        {1980, 1980, 2011, 2013}, {6, 10, 5, 5}, {24, 9, 1, 15}};
    EXPECT_EQ(reader.getData(), gold);
    EXPECT_EQ(reader.getData("column_0"), gold[0]);
    EXPECT_EQ(reader.getData("column_1"), gold[1]);
    EXPECT_EQ(reader.getData("column_2"), gold[2]);
  }

  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_no_header.csv");
    reader.setHeaderFlag(MooseUtils::DelimitedFileReader::HeaderFlag::ON);
    reader.read();

    std::vector<std::vector<double>> gold = {{1980, 2011, 2013}, {10, 5, 5}, {9, 1, 15}};
    EXPECT_EQ(reader.getData(), gold);
    EXPECT_EQ(reader.getData("1980"), gold[0]);
    EXPECT_EQ(reader.getData("6"), gold[1]);
    EXPECT_EQ(reader.getData("24"), gold[2]);
    EXPECT_EQ(reader.getNames(), std::vector<std::string>({"1980", "6", "24"}));
  }
}

TEST(DelimitedFileReader, ExtraSpace)
{
  MooseUtils::DelimitedFileReader reader("data/csv/example_extra_space.csv");
  reader.read();

  std::vector<std::vector<double>> gold = {
      {0, 10, 100, 1000}, {1, 11, 101, 1001}, {2, 12, 102, 1002}};
  EXPECT_EQ(reader.getData(), gold);
  EXPECT_EQ(reader.getData("column_0"), gold[0]);
  EXPECT_EQ(reader.getData("column_1"), gold[1]);
  EXPECT_EQ(reader.getData("column_2"), gold[2]);
}

TEST(DelimitedFileReader, Tabs)
{
  MooseUtils::DelimitedFileReader reader("data/csv/example_tab.csv");
  reader.read();
  EXPECT_EQ(reader.getData(), std::vector<std::vector<double>>({{-500, -499}, {0.023, 0.024}}));
}

TEST(DelimitedFileReader, Scientific)
{
  MooseUtils::DelimitedFileReader reader("data/csv/example_sci.csv");
  reader.read();
  std::vector<std::vector<double>> gold = {{0, 1000, 2000, 5000, 5100},
                                           {0, 20000, 0, 20000, 30000}};
  EXPECT_EQ(reader.getData(), gold);
}

TEST(DelimitedFileReader, Empty)
{
  MooseUtils::DelimitedFileReader reader("data/csv/example_empty.csv");
  reader.read();
  EXPECT_EQ(reader.getData(), std::vector<std::vector<double>>());
}
