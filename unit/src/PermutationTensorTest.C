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
