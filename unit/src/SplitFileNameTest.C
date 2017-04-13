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

#include "MooseUtils.h"

#include <cmath>

TEST(SplitFileName, invalidName)
{
  std::string full = "/this/is/not/valid/";
  EXPECT_ANY_THROW(MooseUtils::splitFileName(full));
}

TEST(SplitFileName, validName)
{
  std::string full = "/this/is/valid.txt";
  std::pair<std::string, std::string> split = MooseUtils::splitFileName(full);

  EXPECT_EQ(split.first.compare("/this/is"), 0);
  EXPECT_EQ(split.second.compare("valid.txt"), 0);

  full = "valid.txt";
  split = MooseUtils::splitFileName(full);
  EXPECT_EQ(split.first.compare("."), 0);
  EXPECT_EQ(split.second.compare("valid.txt"), 0);
}
