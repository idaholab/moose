/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef DELIMITEDFILEREADERTEST_H
#define DELIMITEDFILEREADERTEST_H

#include "gtest/gtest.h"

// MOOSE includes
#include "DelimitedFileReader.h"
#include "MooseException.h"

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
    reader.getColumnData("second");
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

TEST(DelimitedFileReader, Headers)
{
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.read();
    const std::vector<std::string> & cols = reader.getColumnNames();
    std::vector<std::string> gold = {"year", "month", "day"};
    EXPECT_EQ(cols, gold);
  }
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_no_header.csv", /*header=*/false);
    reader.read();
    const std::vector<std::string> & cols = reader.getColumnNames();
    std::vector<std::string> gold = {"column_0", "column_1", "column_2"};
    EXPECT_EQ(cols, gold);
  }
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example_padding_test.csv", /*header=*/false);
    reader.read();
    const std::vector<std::string> & cols = reader.getColumnNames();
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
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.read();
    const std::vector<std::vector<double>> & data = reader.getColumnData();
    std::vector<std::vector<double>> gold = {
        {1980, 1980, 2011, 2013}, {6, 10, 5, 5}, {24, 9, 1, 15}};
    EXPECT_EQ(data, gold);
  }
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.csv");
    reader.read();
    const std::vector<double> & data = reader.getColumnData("day");
    std::vector<double> gold = {24, 9, 1, 15};
    EXPECT_EQ(data, gold);
  }
}

TEST(DelimitedFileReader, DataChangeDelimiter)
{
  {
    MooseUtils::DelimitedFileReader reader("data/csv/example.txt", true, "$");
    reader.read();
    const std::vector<std::vector<double>> & data = reader.getColumnData();
    std::vector<std::vector<double>> gold = {
        {1980, 1980, 2011, 2013}, {6, 10, 5, 5}, {24, 9, 1, 15}};
    EXPECT_EQ(data, gold);
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
    std::string gold = "Failed to convert a delimited data into double when reading row 4 in file "
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

#endif
