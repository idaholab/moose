//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
// Bootstrap CI

#include "Calculators.h"
#include "BootstrapCalculators.h"
#include "MooseEnumItem.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseRandom.h"
#include "NormalDistribution.h"
#include "libmesh/auto_ptr.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_sync.h"

namespace StochasticTools
{

MooseEnum
makeBootstrapCalculatorEnum()
{
  return MooseEnum("percentile=0 bca=1");
}

std::unique_ptr<const BootstrapCalculator>
makeBootstrapCalculator(const MooseEnum & item,
                        const libMesh::ParallelObject & other,
                        const std::vector<Real> & levels,
                        unsigned int replicates,
                        unsigned int seed)
{
  std::unique_ptr<const BootstrapCalculator> ptr = nullptr;
  if (item == "percentile")
    ptr = libmesh_make_unique<const Percentile>(other, levels, replicates, seed);
  else if (item == "bca")
    ptr = libmesh_make_unique<const BiasCorrectedAccelerated>(other, levels, replicates, seed);

  if (!ptr)
    ::mooseError("Failed to create Statistics::BootstrapCalculator object for ", item);

  return ptr;
}

BootstrapCalculator::BootstrapCalculator(const libMesh::ParallelObject & other,
                                         const std::vector<Real> & levels,
                                         unsigned int replicates,
                                         unsigned int seed)
  : libMesh::ParallelObject(other), _levels(levels), _replicates(replicates), _seed(seed)
{
  mooseAssert(*std::min_element(levels.begin(), levels.end()) > 0,
              "The supplied levels must be greater than zero.");
  mooseAssert(*std::max_element(levels.begin(), levels.end()) < 1,
              "The supplied levels must be less than one");
}

std::vector<Real>
BootstrapCalculator::computeBootstrapEstimates(const std::vector<Real> & data,
                                               const Calculator & calc,
                                               const bool is_distributed) const
{
  MooseRandom generator;
  generator.seed(0, _seed);

  // Compute replicate statistics
  std::vector<Real> values(_replicates);
  for (std::size_t i = 0; i < _replicates; ++i)
  {
    std::vector<Real> replicate = shuffle(data, generator, is_distributed);
    values[i] = calc.compute(replicate, is_distributed);
  }
  std::sort(values.begin(), values.end());
  return values;
}

std::vector<Real>
BootstrapCalculator::shuffle(const std::vector<Real> & data,
                             MooseRandom & generator,
                             const bool is_distributed) const
{
  // Size of the local input data
  const std::size_t n_local = data.size();

  // The data to be returned
  std::vector<Real> replicate;

  // REPLICATED data
  if (!is_distributed)
  {
    replicate.resize(n_local);
    for (std::size_t j = 0; j < n_local; ++j)
    {
      auto index = generator.randl(0, 0, n_local);
      replicate[j] = data[index];
    }
  }

  // DISTRIBUTED data
  else
  {
    // Reserve space for storing the local
    replicate.reserve(n_local);

    // Compute the global size of the vector
    std::size_t n_global = n_local;
    _communicator.sum(n_global);

    // Compute the vector data offsets, the scope cleans up the "n_local" vector
    std::vector<std::size_t> offsets(n_processors());
    {
      std::vector<std::size_t> local_sizes;
      _communicator.allgather(n_local, local_sizes);
      for (std::size_t i = 1; i < local_sizes.size(); ++i)
        offsets[i] = offsets[i - 1] + local_sizes[i];
    }

    // Advance the random number generator to the current offset
    for (std::size_t i = 0; i < offsets[processor_id()]; ++i)
      generator.randl(0, 0, n_global);

    // Compute the needs for this processor
    std::unordered_map<processor_id_type, std::vector<std::size_t>> needs;
    for (std::size_t i = 0; i < n_local; ++i)
    {
      auto index = generator.randl(0, 0, n_global);
      // iterator to first element in range that is GREATER THAN index
      auto rank = std::upper_bound(offsets.begin(), offsets.end(), index);
      auto rk = std::distance(offsets.begin(), rank) - 1;
      needs[rk].push_back(index - offsets[rk]);
    }

    // Advance the random number generator to the end of the global vector
    for (std::size_t i = offsets[processor_id()] + n_local; i < n_global; ++i)
      generator.randl(0, 0, n_global);

    // Collect the values to be returned to the various processors
    std::unordered_map<processor_id_type, std::vector<Real>> returns;
    auto return_functor = [&data, &returns](processor_id_type pid,
                                            const std::vector<std::size_t> & indices) {
      auto & returns_pid = returns[pid];
      for (auto idx : indices)
        returns_pid.push_back(data[idx]);
    };
    Parallel::push_parallel_vector_data(_communicator, needs, return_functor);

    // Receive shuffled values from the various processors
    auto recv_functor = [&replicate](processor_id_type /*pid*/, const std::vector<Real> & values) {
      replicate.insert(replicate.end(), values.begin(), values.end());
    };
    Parallel::push_parallel_vector_data(_communicator, returns, recv_functor);
  }

  return replicate;
}

// PERCENTILE //////////////////////////////////////////////////////////////////////////////////////
Percentile::Percentile(const libMesh::ParallelObject & other,
                       const std::vector<Real> & levels,
                       unsigned int replicates,
                       unsigned int seed)
  : BootstrapCalculator(other, levels, replicates, seed)
{
}

std::vector<Real>
Percentile::compute(const std::vector<Real> & data,
                    const Calculator & calc,
                    const bool is_distributed) const
{
  // Bootstrap estimates
  const std::vector<Real> values = computeBootstrapEstimates(data, calc, is_distributed);

  // Extract percentiles
  std::vector<Real> output;
  if (processor_id() == 0)
  {
    for (const Real & level : _levels)
    {
      long unsigned int index = std::lrint(level * (_replicates - 1));
      output.push_back(values[index]);
    }
  }
  return output;
}

// BIASCORRECTEDACCELERATED ////////////////////////////////////////////////////////////////////////
BiasCorrectedAccelerated::BiasCorrectedAccelerated(const libMesh::ParallelObject & other,
                                                   const std::vector<Real> & levels,
                                                   unsigned int replicates,
                                                   unsigned int seed)
  : BootstrapCalculator(other, levels, replicates, seed)
{
}

std::vector<Real>
BiasCorrectedAccelerated::compute(const std::vector<Real> & data,
                                  const Calculator & calc,
                                  const bool is_distributed) const
{
  if (is_distributed)
    mooseError("BiasCorrectedAccelerated does not work with distributed data at this time.");

  // Bootstrap estimates
  const std::vector<Real> values = computeBootstrapEstimates(data, calc, is_distributed);

  // Compute bias-correction, Efron and Tibshirani (2003), Eq. 14.14, p. 186
  const Real value = calc.compute(data, is_distributed);
  const Real count = std::count_if(values.begin(), values.end(), [&value](Real v) {
    return v < value;
  }); // use Real for non-integer divison below
  const Real bias = NormalDistribution::quantile(count / _replicates, 0, 1);

  // Compute Acceleration, Efron and Tibshirani (2003), Eq. 14.15, p. 186
  const Real acc = acceleration(data, calc, is_distributed);

  // Compute intervals, Efron and Tibshirani (2003), Eq. 14.10, p. 185
  std::vector<Real> output;
  for (const Real & level : _levels)
  {
    const Real z = NormalDistribution::quantile(level, 0, 1);
    const Real x = bias + (bias + (bias + z) / (1 - acc * (bias + z)));
    const Real alpha = NormalDistribution::cdf(x, 0, 1);

    long unsigned int index = std::lrint(alpha * (_replicates - 1));
    output.push_back(values[index]);
  }
  return output;
}

Real
BiasCorrectedAccelerated::acceleration(const std::vector<Real> & data,
                                       const Calculator & calc,
                                       const bool is_distributed) const
{
  // Jackknife statistics
  std::vector<Real> theta_i(data.size());

  // Total number of data entries
  Real count = data.size();

  // Compute jackknife estimates, Ch. 11, Eq. 11.2, p. 141
  std::vector<Real> data_not_i(data.size() - 1);
  for (std::size_t i = 0; i < count; ++i)
  {
    std::copy(data.begin(), data.begin() + i, data_not_i.begin());
    std::copy(data.begin() + i + 1, data.end(), data_not_i.begin() + i);
    theta_i[i] = calc.compute(data_not_i, is_distributed);
  }

  // Compute jackknife sum, Ch. 11, Eq. 11.4, p. 141
  Real theta_dot = std::accumulate(theta_i.begin(), theta_i.end(), 0.);
  theta_dot /= count;

  // Acceleration, Ch. 14, Eq. 14.15, p. 185
  Real numerator = 0.;
  Real denomenator = 0;
  for (const auto & jk : theta_i)
  {
    numerator += std::pow(theta_dot - jk, 3);
    denomenator += std::pow(theta_dot - jk, 2);
  }

  mooseAssert(denomenator != 0, "The acceleration denomenator must not be zero.");
  return numerator / (6 * std::pow(denomenator, 3. / 2.));
}
} // namespace
