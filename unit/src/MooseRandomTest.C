//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseRandom.h"
#include <iomanip>
#include <cmath>

class MooseRandomTest : public ::testing::Test
{
protected:
  void SetUp() { MooseRandom::seed(0); }
};

TEST_F(MooseRandomTest, rand)
{
  Real rand_num = MooseRandom::rand();
  EXPECT_NEAR(rand_num, 0.548813502442288, 1e-15);
}

TEST_F(MooseRandomTest, randSeq)
{
  for (unsigned int i = 0; i < 10000; i++)
    MooseRandom::rand();
  Real rand_num = MooseRandom::rand();
  EXPECT_NEAR(rand_num, 0.748267985, 1e-10);
}

TEST_F(MooseRandomTest, randNormal)
{
  Real rand_num = MooseRandom::randNormal();
  EXPECT_NEAR(rand_num, 1.16307809549063, 1e-14);
}

TEST_F(MooseRandomTest, randNormal2)
{
  Real rand_num = MooseRandom::randNormal(0.25, 0.1);
  EXPECT_NEAR(rand_num, 0.366307809549063, 1e-15);
}

TEST_F(MooseRandomTest, randl)
{
  uint32_t rand_num = MooseRandom::randl();
  EXPECT_EQ(rand_num, 2357136044);
}

TEST_F(MooseRandomTest, states)
{
  MooseRandom mrand;

  const unsigned n_gens = 3;
  const unsigned n_nums = 2;

  for (unsigned int i = 0; i < n_gens; ++i)
    mrand.seed(i, i);

  // Save the state so that we can restore the generators
  mrand.saveState();

  std::vector<double> numbers(n_gens * n_nums);

  // Interleave the generators
  for (unsigned int i = 0; i < n_nums; ++i)
    for (unsigned int j = 0; j < n_gens; ++j)
      numbers[i * n_gens + j] = mrand.rand(j);

  // Reset the state
  mrand.restoreState();

  for (unsigned int i = 0; i < n_nums; ++i)
    for (unsigned int j = 0; j < n_gens; ++j)
      EXPECT_NEAR(mrand.rand(j), numbers[i * n_gens + j], 1e-8);
}
