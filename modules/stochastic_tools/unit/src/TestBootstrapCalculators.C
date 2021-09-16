//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include <vector>
#include <cmath>

#include "gtest/gtest.h"
#include "Calculators.h"
#include "BootstrapCalculators.h"
#include "MooseRandom.h"
#include "Normal.h"
#include "libmesh/communicator.h"
#include "libmesh/parallel_object.h"

using namespace StochasticTools;

/**
 * These tests are meant to use the bootstrap calculators and test against analytical
 * confidence intervals. The tolerance for the tests is pretty loose (50%), but at least
 * it shows that the bootstrapping is in the ballpark (as it is meant to).
 *
 * The analytical confidence intervals are based on sampling a normal distribution, this
 * distributiong has nice properties so the confidence interval for mean and standard
 * deviation are analytical.
 *   - For mean, see https://en.wikipedia.org/wiki/Confidence_interval#Basic_steps
 *   - For standard deviation, check out
 * https://stats.stackexchange.com/questions/11707/why-is-sample-standard-deviation-a-biased-estimator-of-sigma/27984#27984
 */

class NormalSampler
{
public:
  NormalSampler(Real mean, Real std, unsigned int seed) : _mean(mean), _std(std)
  {
    _generator.seed(seed);
  }

  Real sample() const { return _generator.randNormal(_mean, _std); }
  std::vector<Real> sample(std::size_t n) const
  {
    std::vector<Real> data(n);
    std::for_each(
        data.begin(), data.end(), [&](Real & v) { v = _generator.randNormal(_mean, _std); });
    return data;
  }

  Real meanConfidence(Real level, std::size_t n) const
  {
    const Real z = computeZ(level);
    const Real mean_std = _std / std::sqrt(n);
    return z * mean_std;
  }

  Real stdConfidence(Real level, std::size_t n) const
  {
    const Real z = computeZ(level);
    const Real nr = n;
    const Real gg = std::exp(std::lgamma(nr / 2.) - std::lgamma((nr - 1.) / 2.));
    const Real std_std = _std * std::sqrt(1.0 - 2. / (nr - 1.) * gg * gg);
    return z * std_std;
  }

private:
  static Real computeZ(Real level)
  {
    const Real alpha = level < 0.5 ? level : 1. - level;
    const Real z = -Normal::quantile(alpha / 2., 0, 1);
    return level < 0.5 ? -z : z;
  }

  const Real _mean;
  const Real _std;
  MooseRandom _generator;
};

TEST(BootstrapCalculators, Percentile)
{
  // Sampling quantities
  const Real mean_dist = 1993;
  const Real std_dist = 27;
  const std::size_t nsamp = 1000;

  // CI quantities
  const unsigned int replicates = 1e4;
  const std::vector<Real> levels = {0.05, 0.1, 0.2, 0.8, 0.9, 0.95};

  // Parallel object to give to calculators
  Parallel::Communicator comm;
  ParallelObject po(comm);

  // Construct mean and standard-deviation calculators
  MultiMooseEnum calc("mean stddev", "mean stddev", true);
  auto mean_calc = makeCalculator(calc[0], po);
  auto std_calc = makeCalculator(calc[1], po);

  // Construct bootstrap calculators
  MooseEnum boot("percentile", "percentile", true);
  auto mean_boot_calc = makeBootstrapCalculator(boot, po, levels, replicates, 2613, *mean_calc);
  auto std_boot_calc = makeBootstrapCalculator(boot, po, levels, replicates, 2613, *std_calc);

  // Construct data and run bootstrapping
  NormalSampler sampler(mean_dist, std_dist, 1945);
  const auto data = sampler.sample(nsamp);
  const Real mean_samp = mean_calc->compute(data, false);
  const Real std_samp = std_calc->compute(data, false);
  const std::vector<Real> mean_ci = mean_boot_calc->compute(data, false);
  const std::vector<Real> std_ci = std_boot_calc->compute(data, false);

  // Compare with reference values
  const Real tol = 5e-1;
  for (const auto & l : index_range(levels))
  {
    const Real mean_ref = sampler.meanConfidence(levels[l], nsamp);
    const Real std_ref = sampler.stdConfidence(levels[l], nsamp);
    EXPECT_NEAR(mean_ci[l] - mean_samp, mean_ref, std::abs(mean_ref * tol));
    EXPECT_NEAR(std_ci[l] - std_samp, std_ref, std::abs(std_ref * tol));
  }
}

TEST(BootstrapCalculators, BiasCorrectedAccelerated)
{
  // Sampling quantities
  const Real mean_dist = 1993;
  const Real std_dist = 27;
  const std::size_t nsamp = 1000;

  // CI quantities
  const unsigned int replicates = 1e4;
  const std::vector<Real> levels = {0.05, 0.1, 0.2, 0.8, 0.9, 0.95};

  // Parallel object to give to calculators
  Parallel::Communicator comm;
  ParallelObject po(comm);

  // Construct mean and standard-deviation calculators
  MultiMooseEnum calc("mean stddev", "mean stddev", true);
  auto mean_calc = makeCalculator(calc[0], po);
  auto std_calc = makeCalculator(calc[1], po);

  // Construct bootstrap calculators
  MooseEnum boot("bca", "bca", true);
  auto mean_boot_calc = makeBootstrapCalculator(boot, po, levels, replicates, 2613, *mean_calc);
  auto std_boot_calc = makeBootstrapCalculator(boot, po, levels, replicates, 2613, *std_calc);

  // Construct data and run bootstrapping
  NormalSampler sampler(mean_dist, std_dist, 1945);
  const auto data = sampler.sample(nsamp);
  const Real mean_samp = mean_calc->compute(data, false);
  const Real std_samp = std_calc->compute(data, false);
  const std::vector<Real> mean_ci = mean_boot_calc->compute(data, false);
  const std::vector<Real> std_ci = std_boot_calc->compute(data, false);

  // Compare with reference values
  const Real tol = 5e-1;
  for (const auto & l : index_range(levels))
  {
    const Real mean_ref = sampler.meanConfidence(levels[l], nsamp);
    const Real std_ref = sampler.stdConfidence(levels[l], nsamp);
    EXPECT_NEAR(mean_ci[l] - mean_samp, mean_ref, std::abs(mean_ref * tol));
    EXPECT_NEAR(std_ci[l] - std_samp, std_ref, std::abs(std_ref * tol));
  }
}

TEST(BootstrapCalculators, Percentile_Vec)
{
  // Sampling quantities
  const std::size_t nsamp = 1000;
  const std::size_t nval = 26;
  std::vector<NormalSampler> samplers;
  for (const auto & k : make_range(nval))
    samplers.emplace_back(/*mean = */ 1993 + 42 * k, /*std = */ 27 + 7 * k, 1945);

  // CI quantities
  const unsigned int replicates = 1e4;
  const std::vector<Real> levels = {0.05, 0.1, 0.2, 0.8, 0.9, 0.95};

  // Parallel object to give to calculators
  Parallel::Communicator comm;
  ParallelObject po(comm);

  // Construct mean and standard-deviation calculators
  MultiMooseEnum calc("mean stddev", "mean stddev", true);
  auto mean_calc = makeCalculator<std::vector<std::vector<Real>>, std::vector<Real>>(calc[0], po);
  auto std_calc = makeCalculator<std::vector<std::vector<Real>>, std::vector<Real>>(calc[1], po);

  // Construct bootstrap calculators
  MooseEnum boot("percentile", "percentile", true);
  auto mean_boot_calc = makeBootstrapCalculator<std::vector<std::vector<Real>>, std::vector<Real>>(
      boot, po, levels, replicates, 2613, *mean_calc);
  auto std_boot_calc = makeBootstrapCalculator<std::vector<std::vector<Real>>, std::vector<Real>>(
      boot, po, levels, replicates, 2613, *std_calc);

  // Construct data and run bootstrapping
  std::vector<std::vector<Real>> data(nsamp);
  for (auto & dt : data)
    for (const auto & samp : samplers)
      dt.push_back(samp.sample());
  const std::vector<Real> mean_samp = mean_calc->compute(data, false);
  const std::vector<Real> std_samp = std_calc->compute(data, false);
  const std::vector<std::vector<Real>> mean_ci = mean_boot_calc->compute(data, false);
  const std::vector<std::vector<Real>> std_ci = std_boot_calc->compute(data, false);

  // Compare with reference values
  const Real tol = 5e-1;
  for (const auto & l : index_range(levels))
    for (const auto & k : make_range(nval))
    {
      const Real mean_ref = samplers[k].meanConfidence(levels[l], nsamp);
      const Real std_ref = samplers[k].stdConfidence(levels[l], nsamp);
      EXPECT_NEAR(mean_ci[l][k] - mean_samp[k], mean_ref, std::abs(mean_ref * tol));
      EXPECT_NEAR(std_ci[l][k] - std_samp[k], std_ref, std::abs(std_ref * tol));
    }
}

TEST(BootstrapCalculators, BiasCorrectedAccelerated_Vec)
{
  // Sampling quantities
  const std::size_t nsamp = 1000;
  const std::size_t nval = 26;
  std::vector<NormalSampler> samplers;
  for (const auto & k : make_range(nval))
    samplers.emplace_back(/*mean = */ 1993 + 42 * k, /*std = */ 27 + 7 * k, 1945);

  // CI quantities
  const unsigned int replicates = 1e4;
  const std::vector<Real> levels = {0.05, 0.1, 0.2, 0.8, 0.9, 0.95};

  // Parallel object to give to calculators
  Parallel::Communicator comm;
  ParallelObject po(comm);

  // Construct mean and standard-deviation calculators
  MultiMooseEnum calc("mean stddev", "mean stddev", true);
  auto mean_calc = makeCalculator<std::vector<std::vector<Real>>, std::vector<Real>>(calc[0], po);
  auto std_calc = makeCalculator<std::vector<std::vector<Real>>, std::vector<Real>>(calc[1], po);

  // Construct bootstrap calculators
  MooseEnum boot("bca", "bca", true);
  auto mean_boot_calc = makeBootstrapCalculator<std::vector<std::vector<Real>>, std::vector<Real>>(
      boot, po, levels, replicates, 2613, *mean_calc);
  auto std_boot_calc = makeBootstrapCalculator<std::vector<std::vector<Real>>, std::vector<Real>>(
      boot, po, levels, replicates, 2613, *std_calc);

  // Construct data and run bootstrapping
  std::vector<std::vector<Real>> data(nsamp);
  for (auto & dt : data)
    for (const auto & samp : samplers)
      dt.push_back(samp.sample());
  const std::vector<Real> mean_samp = mean_calc->compute(data, false);
  const std::vector<Real> std_samp = std_calc->compute(data, false);
  const std::vector<std::vector<Real>> mean_ci = mean_boot_calc->compute(data, false);
  const std::vector<std::vector<Real>> std_ci = std_boot_calc->compute(data, false);

  // Compare with reference values
  const Real tol = 5e-1;
  for (const auto & l : index_range(levels))
    for (const auto & k : make_range(nval))
    {
      const Real mean_ref = samplers[k].meanConfidence(levels[l], nsamp);
      const Real std_ref = samplers[k].stdConfidence(levels[l], nsamp);
      EXPECT_NEAR(mean_ci[l][k] - mean_samp[k], mean_ref, std::abs(mean_ref * tol));
      EXPECT_NEAR(std_ci[l][k] - std_samp[k], std_ref, std::abs(std_ref * tol));
    }
}
