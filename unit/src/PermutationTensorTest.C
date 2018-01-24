//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "PermutationTensor.h"

TEST(PermutationTensor, twoD)
{
  EXPECT_EQ(PermutationTensor::eps(0, 0), 0);
  EXPECT_EQ(PermutationTensor::eps(0, 1), 1);
  EXPECT_EQ(PermutationTensor::eps(1, 0), -1);
  EXPECT_EQ(PermutationTensor::eps(1, 1), 0);
  EXPECT_EQ(PermutationTensor::eps(1, 2), 0);
}

TEST(PermutationTensor, threeD)
{
  EXPECT_EQ(PermutationTensor::eps(0, 0, 0), 0);
  EXPECT_EQ(PermutationTensor::eps(0, 0, 1), 0);
  EXPECT_EQ(PermutationTensor::eps(0, 0, 2), 0);

  EXPECT_EQ(PermutationTensor::eps(0, 1, 0), 0);
  EXPECT_EQ(PermutationTensor::eps(0, 1, 1), 0);
  EXPECT_EQ(PermutationTensor::eps(0, 1, 2), 1);

  EXPECT_EQ(PermutationTensor::eps(0, 2, 0), 0);
  EXPECT_EQ(PermutationTensor::eps(0, 2, 1), -1);
  EXPECT_EQ(PermutationTensor::eps(0, 2, 2), 0);

  EXPECT_EQ(PermutationTensor::eps(1, 0, 0), 0);
  EXPECT_EQ(PermutationTensor::eps(1, 0, 1), 0);
  EXPECT_EQ(PermutationTensor::eps(1, 0, 2), -1);

  EXPECT_EQ(PermutationTensor::eps(1, 1, 0), 0);
  EXPECT_EQ(PermutationTensor::eps(1, 1, 1), 0);
  EXPECT_EQ(PermutationTensor::eps(1, 1, 2), 0);

  EXPECT_EQ(PermutationTensor::eps(1, 2, 0), 1);
  EXPECT_EQ(PermutationTensor::eps(1, 2, 1), 0);
  EXPECT_EQ(PermutationTensor::eps(1, 2, 2), 0);

  EXPECT_EQ(PermutationTensor::eps(2, 0, 0), 0);
  EXPECT_EQ(PermutationTensor::eps(2, 0, 1), 1);
  EXPECT_EQ(PermutationTensor::eps(2, 0, 2), 0);

  EXPECT_EQ(PermutationTensor::eps(2, 1, 0), -1);
  EXPECT_EQ(PermutationTensor::eps(2, 1, 1), 0);
  EXPECT_EQ(PermutationTensor::eps(2, 1, 2), 0);

  EXPECT_EQ(PermutationTensor::eps(2, 2, 0), 0);
  EXPECT_EQ(PermutationTensor::eps(2, 2, 1), 0);
  EXPECT_EQ(PermutationTensor::eps(2, 2, 2), 0);

  EXPECT_EQ(PermutationTensor::eps(1, 2, 3), 0);
}

// note that i don't test all permutations in fourD because
// i'm lazy, but also because i doubt 4D will ever be used.
TEST(PermutationTensor, fourD)
{
  EXPECT_EQ(PermutationTensor::eps(0, 1, 2, 3), 1);
  EXPECT_EQ(PermutationTensor::eps(0, 1, 3, 2), -1);
  EXPECT_EQ(PermutationTensor::eps(0, 2, 1, 3), -1);
  EXPECT_EQ(PermutationTensor::eps(0, 2, 3, 1), 1);
  EXPECT_EQ(PermutationTensor::eps(0, 3, 1, 2), 1);
  EXPECT_EQ(PermutationTensor::eps(0, 3, 2, 1), -1);

  EXPECT_EQ(PermutationTensor::eps(1, 0, 2, 3), -1);
  EXPECT_EQ(PermutationTensor::eps(1, 0, 3, 2), 1);
  EXPECT_EQ(PermutationTensor::eps(1, 2, 0, 3), 1);
  EXPECT_EQ(PermutationTensor::eps(1, 2, 3, 0), -1);
  EXPECT_EQ(PermutationTensor::eps(1, 3, 0, 2), -1);
  EXPECT_EQ(PermutationTensor::eps(1, 3, 2, 0), 1);

  EXPECT_EQ(PermutationTensor::eps(2, 0, 1, 3), 1);
  EXPECT_EQ(PermutationTensor::eps(2, 0, 3, 1), -1);
  EXPECT_EQ(PermutationTensor::eps(2, 1, 0, 3), -1);
  EXPECT_EQ(PermutationTensor::eps(2, 1, 3, 0), 1);
  EXPECT_EQ(PermutationTensor::eps(2, 3, 0, 1), 1);
  EXPECT_EQ(PermutationTensor::eps(2, 3, 1, 0), -1);

  EXPECT_EQ(PermutationTensor::eps(3, 0, 1, 2), -1);
  EXPECT_EQ(PermutationTensor::eps(3, 0, 2, 1), 1);
  EXPECT_EQ(PermutationTensor::eps(3, 1, 0, 2), 1);
  EXPECT_EQ(PermutationTensor::eps(3, 1, 2, 0), -1);
  EXPECT_EQ(PermutationTensor::eps(3, 2, 0, 1), -1);
  EXPECT_EQ(PermutationTensor::eps(3, 2, 1, 0), 1);
}
