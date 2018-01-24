//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

int
getLocation(const std::vector<unsigned int> & offsets, unsigned int global_index)
{

  // This works
  std::vector<unsigned int>::const_iterator iter =
      std::upper_bound(offsets.begin(), offsets.end(), global_index) - 1;

  // This no worky
  // std::vector<unsigned int>::const_iterator iter =
  //  std::lower_bound(offsets.begin(), offsets.end(), global_index);

  return std::distance(offsets.begin(), iter);
}

TEST(Sampler, GetLocation)
{
  std::vector<unsigned int> offsets = {0, 4};
  EXPECT_EQ(getLocation(offsets, 0), 0); // lower: 0
  EXPECT_EQ(getLocation(offsets, 1), 0); // lower: 1
  EXPECT_EQ(getLocation(offsets, 2), 0); // lower: 1
  EXPECT_EQ(getLocation(offsets, 3), 0); // lower: 1
  EXPECT_EQ(getLocation(offsets, 4), 1); // lower: 1
  EXPECT_EQ(getLocation(offsets, 5), 1); // lower: 2
  EXPECT_EQ(getLocation(offsets, 6), 1); // lower: 2
  EXPECT_EQ(getLocation(offsets, 7), 1); // lower: 2
  EXPECT_EQ(getLocation(offsets, 8), 1); // lower: 2
  EXPECT_EQ(getLocation(offsets, 9), 1); // lower: 2
}
