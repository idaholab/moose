//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
