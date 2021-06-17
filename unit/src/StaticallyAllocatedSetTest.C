//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "StaticallyAllocatedSet.h"

TEST(StaticallyAllocatedSet, test)
{
  MooseUtils::StaticallyAllocatedSet<int, 4> set;

  EXPECT_EQ(set.size(), 0);
  EXPECT_EQ(set.dataEndPos(), 0);

  set.insert(2);
  EXPECT_EQ(set.dataEndPos(), 1);
  EXPECT_EQ(set.contains(2), true);

  set.insert(1);
  EXPECT_EQ(set.dataEndPos(), 2);
  EXPECT_EQ(set.contains(1), true);

  set.clear();
  EXPECT_EQ(set.dataEndPos(), 0);
  EXPECT_EQ(set.contains(2), false);
  EXPECT_EQ(set.contains(1), false);

  set.insert(3);
  set.insert(4);

  MooseUtils::StaticallyAllocatedSet<int, 4> set2;
  set2.insert(5);

  set.swap(set2);

  EXPECT_EQ(set.contains(3), false);
  EXPECT_EQ(set.contains(4), false);
  EXPECT_EQ(set.contains(5), true);
  EXPECT_EQ(set.size(), 1);

  EXPECT_EQ(set2.contains(3), true);
  EXPECT_EQ(set2.contains(4), true);
  EXPECT_EQ(set2.contains(5), false);
  EXPECT_EQ(set2.size(), 2);
}
