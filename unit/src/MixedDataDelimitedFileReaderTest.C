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


