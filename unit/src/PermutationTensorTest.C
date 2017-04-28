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

#include "RankTwoTensor.h"
#include "RankThreeTensor.h"
#include "RankFourTensor.h"

TEST(leviCivita, twoD)
{
  EXPECT_EQ(RankTwoTensor::leviCivita(0, 0), 0);
  EXPECT_EQ(RankTwoTensor::leviCivita(0, 1), 1);
  EXPECT_EQ(RankTwoTensor::leviCivita(1, 0), -1);
  EXPECT_EQ(RankTwoTensor::leviCivita(1, 1), 0);
  EXPECT_EQ(RankTwoTensor::leviCivita(1, 2), 0);
}

TEST(leviCivita, threeD)
{
  EXPECT_EQ(RankThreeTensor::leviCivita(0, 0, 0), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(0, 0, 1), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(0, 0, 2), 0);

  EXPECT_EQ(RankThreeTensor::leviCivita(0, 1, 0), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(0, 1, 1), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(0, 1, 2), 1);

  EXPECT_EQ(RankThreeTensor::leviCivita(0, 2, 0), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(0, 2, 1), -1);
  EXPECT_EQ(RankThreeTensor::leviCivita(0, 2, 2), 0);

  EXPECT_EQ(RankThreeTensor::leviCivita(1, 0, 0), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(1, 0, 1), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(1, 0, 2), -1);

  EXPECT_EQ(RankThreeTensor::leviCivita(1, 1, 0), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(1, 1, 1), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(1, 1, 2), 0);

  EXPECT_EQ(RankThreeTensor::leviCivita(1, 2, 0), 1);
  EXPECT_EQ(RankThreeTensor::leviCivita(1, 2, 1), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(1, 2, 2), 0);

  EXPECT_EQ(RankThreeTensor::leviCivita(2, 0, 0), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(2, 0, 1), 1);
  EXPECT_EQ(RankThreeTensor::leviCivita(2, 0, 2), 0);

  EXPECT_EQ(RankThreeTensor::leviCivita(2, 1, 0), -1);
  EXPECT_EQ(RankThreeTensor::leviCivita(2, 1, 1), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(2, 1, 2), 0);

  EXPECT_EQ(RankThreeTensor::leviCivita(2, 2, 0), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(2, 2, 1), 0);
  EXPECT_EQ(RankThreeTensor::leviCivita(2, 2, 2), 0);

  EXPECT_EQ(RankThreeTensor::leviCivita(1, 2, 3), 0);
}

// note that we don't test all permutations in fourD because
// we're lazy, but also because we doubt 4D will ever be used.
TEST(leviCivita, fourD)
{
  EXPECT_EQ(RankFourTensor::leviCivita(0, 1, 2, 3), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(0, 1, 3, 2), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(0, 2, 1, 3), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(0, 2, 3, 1), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(0, 3, 1, 2), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(0, 3, 2, 1), -1);

  EXPECT_EQ(RankFourTensor::leviCivita(1, 0, 2, 3), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(1, 0, 3, 2), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(1, 2, 0, 3), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(1, 2, 3, 0), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(1, 3, 0, 2), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(1, 3, 2, 0), 1);

  EXPECT_EQ(RankFourTensor::leviCivita(2, 0, 1, 3), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(2, 0, 3, 1), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(2, 1, 0, 3), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(2, 1, 3, 0), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(2, 3, 0, 1), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(2, 3, 1, 0), -1);

  EXPECT_EQ(RankFourTensor::leviCivita(3, 0, 1, 2), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(3, 0, 2, 1), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(3, 1, 0, 2), 1);
  EXPECT_EQ(RankFourTensor::leviCivita(3, 1, 2, 0), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(3, 2, 0, 1), -1);
  EXPECT_EQ(RankFourTensor::leviCivita(3, 2, 1, 0), 1);
}
