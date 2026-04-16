//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseRandomPerturbation.h"

#include "randistrs.h"

/// Tests valid permutation
TEST(MooseRandomPerturbation, permute)
{
  unsigned int num_trials = 10;
  unsigned int max_vector_length = 1000;
  unsigned int seed = 1993;

  mt_state state;
  mts_seed32new(&state, seed);
  for (unsigned int trial = 0; trial < num_trials; ++trial)
  {
    auto seed = static_cast<unsigned long>(mts_lrand(&state));
    unsigned int n = rds_iuniform(&state, 0, max_vector_length);
    MooseRandomPerturbation generator(seed, n);

    std::vector<unsigned int> permutation(n);
    for (unsigned int x = 0; x < n; ++x)
      permutation[x] = generator.permute(x);

    std::vector<unsigned int> sorted_perm = permutation;
    std::sort(sorted_perm.begin(), sorted_perm.end());
    for (unsigned int x = 0; x < n; ++x)
      EXPECT_EQ(sorted_perm[x], x);
  }
}

/// Test that the permutation is invertible
TEST(MooseRandomPerturbation, invert)
{
  unsigned int num_trials = 10;
  unsigned int max_vector_length = 1000;
  unsigned int seed = 1993;

  mt_state state;
  mts_seed32new(&state, seed);
  for (unsigned int trial = 0; trial < num_trials; ++trial)
  {
    auto generator_seed = static_cast<unsigned long>(mts_lrand(&state));
    unsigned int n = rds_iuniform(&state, 0, max_vector_length);
    MooseRandomPerturbation generator(generator_seed, n);

    std::vector<unsigned int> permutation(n);
    for (unsigned int x = 0; x < n; ++x)
    {
      auto y = generator.permute(x);
      auto xinv = generator.invert(y);
      EXPECT_EQ(xinv, x);
    }
  }
}

/// Tests that permutation works over a range of bits
TEST(MooseRandomPerturbation, bits)
{
  unsigned int max_bits = 29; // Can go up to 32, but is slow
  unsigned int num_trials = 10;
  unsigned int seed = 1993;

  mt_state state;
  mts_seed32new(&state, seed);
  unsigned int max_n = 0;
  for (unsigned int bits = 1; bits <= max_bits; ++bits)
  {
    auto min_n = max_n + 1;
    max_n = static_cast<unsigned int>((uint64_t(1) << bits) - 1);

    auto generator_seed = static_cast<unsigned long>(mts_lrand(&state));
    unsigned int n = rds_iuniform(&state, min_n, max_n);
    MooseRandomPerturbation generator(generator_seed, n);

    for (unsigned int trial = 0; trial < num_trials; ++trial)
    {
      uint32_t x = rds_iuniform(&state, 0, n);
      auto y = generator.permute(x);
      auto xinv = generator.invert(y);
      EXPECT_EQ(xinv, x);
    }
  }
}

/// Tests that permutations are unique for a given seed
TEST(MooseRandomPerturbation, unique)
{
  unsigned int num_trials = 10;
  unsigned int n = 1000;
  unsigned int seed = 1993;

  mt_state state;
  mts_seed32new(&state, seed);
  std::vector<std::vector<uint32_t>> permutations(num_trials, std::vector<uint32_t>(n));
  for (unsigned int trial = 0; trial < num_trials; ++trial)
  {
    auto seed = static_cast<unsigned long>(mts_lrand(&state));
    MooseRandomPerturbation generator(seed, n);

    for (unsigned int x = 0; x < n; ++x)
      permutations[trial][x] = generator.permute(x);

    for (unsigned int test = 0; test < trial; ++test)
      EXPECT_NE(permutations[trial], permutations[test]);
  }
}

/// Tests permutations are reproducible
TEST(MooseRandomPerturbation, reproduce)
{
  unsigned int num_trials = 10;
  unsigned int max_vector_length = 1000;
  unsigned int num_tests = 100;
  unsigned int seed = 1993;

  mt_state state;
  mts_seed32new(&state, seed);
  for (unsigned int trial = 0; trial < num_trials; ++trial)
  {
    auto seed = static_cast<unsigned long>(mts_lrand(&state));
    unsigned int n = rds_iuniform(&state, 0, max_vector_length);
    MooseRandomPerturbation generator(seed, n);

    std::vector<unsigned int> permutation(n);
    for (unsigned int x = 0; x < n; ++x)
      permutation[x] = generator.permute(x);

    for (unsigned int test = 0; test < num_tests; ++test)
    {
      uint32_t x = rds_iuniform(&state, 0, n);
      EXPECT_EQ(permutation[x], generator.permute(x));
    }
  }
}
