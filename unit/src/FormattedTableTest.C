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
    ASSERT_NE(msg.find("Failed to convert 'foo' to the supplied type of"), std::string::npos)
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
    ASSERT_NE(msg.find("Failed to convert '12345foo' to the supplied type of"), std::string::npos)
        << "failed with unexpected error: " << msg;
  }
}
