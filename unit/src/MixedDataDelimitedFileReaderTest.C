//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MixedDataDelimitedFileReader.h"

TEST(MixedDataDelimitedFileReader, ColumnByName)
{
  std::vector<double> gold = {1980, 1980, 2011, 2013};

  MooseUtils::MixedDataDelimitedFileReader reader("data/csv/example.csv");
  reader.read();
  const auto data = reader.getData<double>("year");
  EXPECT_EQ(data, gold);
}

TEST(MixedDataDelimitedFileReader, ColumnByIndex)
{
  std::vector<double> gold = {1980, 1980, 2011, 2013};

  MooseUtils::MixedDataDelimitedFileReader reader("data/csv/example.csv");
  reader.read();
  const auto data = reader.getData<double>(0);
  EXPECT_EQ(data, gold);
}

TEST(MixedDataDelimitedFileReader, MixedDataCSV)
{
  std::vector<double> gold = {1980, 1980, 2011, 2013};

  MooseUtils::MixedDataDelimitedFileReader reader("data/csv/example_mixed_data.csv");
  reader.read();
  auto data = reader.getData<double>("year");
  EXPECT_EQ(data, gold);

  data = reader.getData<double>(0);
  EXPECT_EQ(data, gold);
}
