//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
// Bootstrap CI

#include "Statistics.h"
#include "BootstrapStatistics.h"
#include "MooseEnumItem.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseRandom.h"
#include "libmesh/auto_ptr.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_sync.h"

#include "SerializerGuard.h"

namespace Statistics
{

MooseEnum
makeBootstrapCalculatorEnum()
{
  return MooseEnum("percentile=0");
}

std::unique_ptr<const BootstrapCalculator>
makeBootstrapCalculator(const MooseEnum & item,
                        const MooseObject & other,
                        const std::vector<Real> & levels,
                        unsigned int replicates,
                        unsigned int seed)
{

  std::unique_ptr<BootstrapCalculator> ptr = nullptr;
  if (item == "percentile")
    ptr = libmesh_make_unique<Percentile>(other);

  ptr->setLevels(levels);
  ptr->setReplicates(replicates);
  ptr->setSeed(seed);
  if (!ptr)
    ::mooseError("Failed to create Statistics::BootstrapCalculator object for ", item);
  return ptr;
}

BootstrapCalculator::BootstrapCalculator(const MooseObject & other) : libMesh::ParallelObject(other)
{
}

void
BootstrapCalculator::setLevels(std::vector<Real> levels)
{
  mooseAssert(*std::min_element(levels.begin(), levels.end()) > 0,
              "The supplied levels must be greater than zero.");
  mooseAssert(*std::max_element(levels.begin(), levels.end()) < 1,
              "The supplied levels must be less than one");
  _levels = levels;
}

void
BootstrapCalculator::setReplicates(const unsigned int replicates)
{
  _replicates = replicates;
}

void
BootstrapCalculator::setSeed(const unsigned int seed)
{
  _seed = seed;
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
Percentile::Percentile(const MooseObject & other) : BootstrapCalculator(other) {}

std::vector<Real>
Percentile::compute(const std::vector<Real> & data,
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

  // Extract percentiles
  std::vector<Real> output;
  for (const Real & level : _levels)
  {
    long unsigned int index = std::lrint(level * (_replicates - 1));
    output.push_back(values[index]);
  }

  return output;
}

} // namespace
