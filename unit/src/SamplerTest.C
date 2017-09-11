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
