//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "gtest/gtest.h"

#include "MooseRandomStateless.h"

std::vector<uint32_t>
randomIndices(std::size_t num, std::size_t max_i)
{
  mt_state state;
  mts_seed32new(&state, 2025);
  std::vector<uint32_t> indices(num);
  for (auto & i : indices)
    i = rds_iuniform(&state, 0, max_i);
  return indices;
}

TEST(MooseRandomStatelessGenerator, rand)
{
  unsigned int seed = 1993;
  // This will ensure at lest one test goes beyond MT_STATE_SIZE
  std::size_t num_samples = 10000;
  std::size_t num_tests = 100;

  mt_state state;
  mts_seed32new(&state, seed);
  std::vector<Real> gold(num_samples);
  for (auto & v : gold)
    v = mts_ldrand(&state);

  MooseRandomStateless::Generator<double> rng(mts_ldrand, seed);
  for (const auto ind : randomIndices(num_tests, num_samples))
    EXPECT_NEAR(rng.evaluate(ind), gold[ind], 1e-12);
}

TEST(MooseRandomStatelessGenerator, randl)
{
  unsigned int seed = 1993;
  std::size_t num_samples = 100;
  std::size_t num_tests = 100;

  mt_state state;
  mts_seed32new(&state, seed);
  std::vector<uint32_t> gold(num_samples);
  for (auto & v : gold)
    v = mts_lrand(&state);

  MooseRandomStateless::Generator<uint32_t> rng(mts_lrand, seed);
  for (const auto ind : randomIndices(num_tests, num_samples))
    EXPECT_EQ(rng.evaluate(ind), gold[ind]);
}

TEST(MooseRandomStatelessGenerator, randlb)
{
  unsigned int seed = 1993;
  std::size_t num_samples = 100;
  std::size_t num_tests = 100;
  unsigned int lb = 0;
  unsigned int ub = 1e6;

  mt_state state;
  mts_seed32new(&state, seed);
  std::vector<uint32_t> gold(num_samples);
  for (auto & v : gold)
    v = rds_iuniform(&state, lb, ub);

  auto rng_func = [&lb, &ub](mt_state * state) { return rds_iuniform(state, lb, ub); };
  MooseRandomStateless::Generator<uint32_t> rng(rng_func, seed);

  for (const auto ind : randomIndices(num_tests, num_samples))
    EXPECT_EQ(rng.evaluate(ind), gold[ind]);
}

TEST(MooseRandomStatelessGenerator, advance)
{
  unsigned int seed = 1993;
  std::size_t num_samples = 100;
  std::size_t num_tests = 100;
  const unsigned int first_batch_size = 44;
  const unsigned int second_batch_size = num_samples - first_batch_size;

  mt_state state;
  mts_seed32new(&state, seed);
  std::vector<Real> gold(num_samples);
  for (auto & v : gold)
    v = mts_ldrand(&state);

  MooseRandomStateless::Generator<double> rng(mts_ldrand, seed);
  auto random_indices = randomIndices(num_tests, first_batch_size);
  for (const auto ind : random_indices)
    EXPECT_NEAR(rng.evaluate(ind), gold[ind], 1e-12);

  rng.advance(first_batch_size);
  random_indices = randomIndices(num_tests, second_batch_size);
  for (const auto ind : random_indices)
    EXPECT_NEAR(rng.evaluate(ind), gold[first_batch_size + ind], 1e-12);
}

TEST(MooseRandomStateless, rand)
{
  const unsigned int num_trials = 10;
  const std::size_t num_samples = 100;
  const std::size_t num_tests = 100;

  std::vector<Real> gold(num_samples);
  const auto indices = randomIndices(num_tests, num_samples);

  mt_state seed_state;
  mts_seed32new(&seed_state, 1993);
  mt_state state;
  for ([[maybe_unused]] const auto i : make_range(num_trials))
  {
    const unsigned int seed = mts_lrand(&seed_state);
    mts_seed32new(&state, seed);
    MooseRandomStateless rng(seed);

    for (auto & v : gold)
      v = mts_ldrand(&state);

    for (const auto ind : indices)
      EXPECT_NEAR(rng.rand(ind), gold[ind], 1e-12);
  }
}

TEST(MooseRandomStateless, randl)
{
  const unsigned int num_trials = 10;
  const std::size_t num_samples = 100;
  const std::size_t num_tests = 100;

  std::vector<uint32_t> gold(num_samples);
  const auto indices = randomIndices(num_tests, num_samples);

  mt_state seed_state;
  mts_seed32new(&seed_state, 1993);
  mt_state state;
  for ([[maybe_unused]] const auto i : make_range(num_trials))
  {
    const unsigned int seed = mts_lrand(&seed_state);
    mts_seed32new(&state, seed);
    MooseRandomStateless rng(seed);

    for (auto & v : gold)
      v = mts_lrand(&state);

    for (const auto ind : indices)
      EXPECT_EQ(rng.randl(ind), gold[ind]);
  }
}

TEST(MooseRandomStateless, randlb)
{
  const unsigned int num_trials = 10;
  const std::size_t num_samples = 100;
  const std::size_t num_tests = 100;
  const std::size_t num_bounds = 10;

  std::vector<uint32_t> gold(num_samples * num_bounds);
  const auto indices = randomIndices(num_tests, num_samples);

  mt_state seed_state;
  mts_seed32new(&seed_state, 1993);
  mt_state initial_state;
  for ([[maybe_unused]] const auto i : make_range(num_trials))
  {
    const unsigned int seed = mts_lrand(&seed_state);
    mts_seed32new(&initial_state, seed);
    MooseRandomStateless rng(seed);

    // Get an assortment of upper and lower bounds
    std::vector<std::pair<unsigned int, unsigned int>> bounds(num_bounds);
    for (const auto bi : make_range(num_bounds))
    {
      // Get a random upper and lower bound
      const unsigned int lb = rds_iuniform(&seed_state, 0, 1e6);
      const unsigned int ub = lb + (unsigned int)rds_iuniform(&seed_state, 1, 1e6);
      bounds[bi] = std::make_pair(lb, ub);

      // Create num_samples gold values for each bound
      mt_state state = initial_state;
      for (const auto si : make_range(num_samples))
        gold[si * num_bounds + bi] = rds_iuniform(&state, lb, ub);
    }

    for (const auto ind : indices)
    {
      // Get a random pair of bounds
      const std::size_t bi = rds_iuniform(&seed_state, 0, num_bounds);
      const auto [lb, ub] = bounds[bi];

      EXPECT_EQ(rng.randl(ind, lb, ub), gold[ind * num_bounds + bi]);
    }
  }
}

TEST(MooseRandomStateless, mixed)
{
  const unsigned int num_trials = 10;
  const std::size_t num_samples = 100;
  const std::size_t num_tests = 100;

  std::vector<Real> gold_rand(num_samples);
  std::vector<unsigned int> gold_randl(num_samples);
  std::vector<unsigned int> gold_randlb(num_samples);
  const auto indices = randomIndices(num_tests, num_samples);

  mt_state seed_state;
  mts_seed32new(&seed_state, 1993);
  mt_state initial_state;
  for ([[maybe_unused]] const auto i : make_range(num_trials))
  {
    const unsigned int seed = mts_lrand(&seed_state);
    mts_seed32new(&initial_state, seed);
    MooseRandomStateless rng(seed);

    mt_state state = initial_state;
    for (auto & v : gold_rand)
      v = mts_ldrand(&state);

    state = initial_state;
    for (auto & v : gold_randl)
      v = mts_lrand(&state);

    // Create some arbitrary bounds for the bounded randl
    const unsigned int lb = rds_iuniform(&seed_state, 0, 1e6);
    const unsigned int ub = lb + rds_iuniform(&seed_state, 1, 1e6);
    state = initial_state;
    for (auto & v : gold_randlb)
      v = rds_iuniform(&state, lb, ub);

    for (const auto ind : indices)
    {
      // Get a random mix of random number types: 0: rand, 1: randl, 2: bounded randl
      const auto rt = rds_iuniform(&seed_state, 0, 3);
      if (rt == 0)
        EXPECT_NEAR(rng.rand(ind), gold_rand[ind], 1e-12);
      else if (rt == 1)
        EXPECT_EQ(rng.randl(ind), gold_randl[ind]);
      else if (rt == 2)
        EXPECT_EQ(rng.randl(ind, lb, ub), gold_randlb[ind]);
      else
        mooseError("");
    }
  }
}

TEST(MooseRandomStateless, advance)
{
  unsigned int seed = 1993;
  std::size_t num_samples = 100;
  std::size_t num_tests = 100;
  const unsigned int first_batch_size = 44;
  const unsigned int second_batch_size = num_samples - first_batch_size;

  mt_state initial_state, state;
  mts_seed32new(&initial_state, seed);
  MooseRandomStateless rng(seed);
  rng.advance(first_batch_size);

  const auto random_indices = randomIndices(num_tests, second_batch_size);

  // rand
  {
    state = initial_state;
    std::vector<Real> gold(num_samples);
    for (auto & v : gold)
      v = mts_ldrand(&state);
    for (const auto ind : random_indices)
      EXPECT_NEAR(rng.rand(ind), gold[first_batch_size + ind], 1e-12);
  }

  // randl
  {
    state = initial_state;
    std::vector<unsigned int> gold(num_samples);
    for (auto & v : gold)
      v = mts_lrand(&state);
    for (const auto ind : random_indices)
      EXPECT_EQ(rng.randl(ind), gold[first_batch_size + ind]);
  }

  // Bounded randl
  {
    unsigned int lb = 0;
    unsigned int ub = 1e6;

    state = initial_state;
    std::vector<unsigned int> gold(num_samples);
    for (auto & v : gold)
      v = rds_iuniform(&state, lb, ub);
    for (const auto ind : random_indices)
      EXPECT_EQ(rng.randl(ind, lb, ub), gold[first_batch_size + ind]);

    // Make sure new bounds will honor the advance
    ub = 2e6;
    state = initial_state;
    for (auto & v : gold)
      v = rds_iuniform(&state, lb, ub);
    for (const auto ind : random_indices)
      EXPECT_EQ(rng.randl(ind, lb, ub), gold[first_batch_size + ind]);
  }
}

TEST(MooseRandomStateless, restore)
{
  unsigned int seed = 1993;
  std::size_t num_samples = 100;
  std::size_t num_tests = 100;

  mt_state seed_state;
  mts_seed32new(&seed_state, seed);

  std::vector<std::unique_ptr<MooseRandomStateless>> old_rngs(3);
  old_rngs[0] = std::make_unique<MooseRandomStateless>(mts_lrand(&seed_state));
  old_rngs[1] = std::make_unique<MooseRandomStateless>(mts_lrand(&seed_state));
  old_rngs[2] = std::make_unique<MooseRandomStateless>(mts_lrand(&seed_state));
  old_rngs[0]->advance(37);
  old_rngs[1]->advance(44);

  std::ostringstream oss;
  dataStore(oss, old_rngs, Moose::AnyPointer(this));

  std::vector<std::unique_ptr<MooseRandomStateless>> new_rngs;
  std::istringstream iss(oss.str());
  dataLoad(iss, new_rngs, Moose::AnyPointer(this));

  for (const auto i : index_range(old_rngs))
  {
    const auto & old_rng = *old_rngs[i];
    const auto & new_rng = *new_rngs[i];
    for (const auto ind : randomIndices(num_tests, num_samples))
    {
      EXPECT_NEAR(old_rng.rand(ind), new_rng.rand(ind), 1e-12);
      EXPECT_EQ(old_rng.randl(ind), new_rng.randl(ind));

      unsigned int lb = rds_iuniform(&seed_state, 0, 1e6);
      unsigned int ub = lb + rds_iuniform(&seed_state, 1, 1e6);
      EXPECT_EQ(old_rng.randl(ind, lb, ub), new_rng.randl(ind, lb, ub));
    }
  }
}
