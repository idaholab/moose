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
    ASSERT_NE(msg.find("Failed to convert 'foo' to an int."), std::string::npos)
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
    ASSERT_NE(msg.find("Failed to convert '12345foo' to an int."), std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}
