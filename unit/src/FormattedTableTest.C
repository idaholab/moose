//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"
#include "FormattedTable.h"
#include "MooseEnum.h"

TEST(FormattedTable, printTableErrors)
{
  FormattedTable table;
  std::ostringstream oss;
  MooseEnum width("ENVIRONMENT AUTO", "", true);

  try
  {
    width = "foo";
    table.printTable(oss, 0, width);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Unable to convert 'foo' to type int"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }

  try
  {
    width = "12345foo";
    table.printTable(oss, 0, width);
  }
  catch (const std::exception & e)
  {
    std::string msg(e.what());
    ASSERT_NE(msg.find("Unable to convert '12345foo' to type int"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}

TEST(FormattedTable, printTable)
{
  std::vector<std::string> col_names = {"bool", "int", "real", "string"};
  std::vector<bool> col1 = {true, true, false, false};
  std::vector<int> col2 = {1, -2, 3, 4};
  std::vector<Real> col3 = {5.5, 6.66, 7.7, 8.8};
  std::vector<std::string> col4 = {"nine", "ten", "eleven", "twelve"};

  std::ostringstream oss_expect;
  oss_expect
      << "+----------------+----------------+----------------+----------------+----------------+\n"
      << "| time           | bool           | int            | real           | string         |\n"
      << "+----------------+----------------+----------------+----------------+----------------+\n"
      << "|   0.000000e+00 |           True |              1 |   5.500000e+00 |           nine |\n"
      << "|   1.000000e+00 |           True |             -2 |   6.660000e+00 |            ten |\n"
      << "|   2.000000e+00 |          False |              3 |   7.700000e+00 |         eleven |\n"
      << "|   3.000000e+00 |          False |              4 |   8.800000e+00 |         twelve |\n"
      << "+----------------+----------------+----------------+----------------+----------------+\n";

  {
    FormattedTable table;
    for (unsigned int i = 0; i < 4; ++i)
    {
      table.addRow(i);
      table.addData<bool>(col_names[0], col1[i]);
      table.addData<int>(col_names[1], col2[i]);
      table.addData(col_names[2], col3[i]);
      table.addData<std::string>(col_names[3], col4[i]);
    }

    std::ostringstream oss;
    table.printTable(oss);

    EXPECT_EQ(oss_expect.str(), oss.str());
  }

  {
    FormattedTable table;
    table.addData<bool>(col_names[0], col1);
    table.addData<int>(col_names[1], col2);
    table.addData(col_names[2], col3);
    table.addData<std::string>(col_names[3], col4);

    std::ostringstream oss;
    table.printTable(oss);

    EXPECT_EQ(oss_expect.str(), oss.str());
  }
}
