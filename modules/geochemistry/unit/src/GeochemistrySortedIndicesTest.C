//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "GeochemistrySortedIndices.h"

TEST(GeochemistrySortedIndicesTest, sortedIndices)
{
  std::vector<Real> vec = {1, 2, 5, 3, -2};

  std::vector<unsigned> ind = GeochemistrySortedIndices::sortedIndices(vec, true);
  std::vector<unsigned> gold = {4, 0, 1, 3, 2};
  for (unsigned i = 0; i < 5; ++i)
    EXPECT_EQ(ind[i], gold[i]);

  ind = GeochemistrySortedIndices::sortedIndices(vec, false);
  gold = {2, 3, 1, 0, 4};
  for (unsigned i = 0; i < 5; ++i)
    EXPECT_EQ(ind[i], gold[i]);
}
