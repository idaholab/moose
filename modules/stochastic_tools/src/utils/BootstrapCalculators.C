//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
// Bootstrap CI
#include "libmesh/auto_ptr.h"
#include "libmesh/parallel.h"
#include "libmesh/parallel_sync.h"

#include "Calculators.h"
#include "BootstrapCalculators.h"
#include "Shuffle.h"
#include "MooseEnumItem.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseRandom.h"
#include "NormalDistribution.h"

namespace StochasticTools
{

MooseEnum
makeBootstrapCalculatorEnum()
{
  return MooseEnum("percentile=0 bca=1");
}

template <typename InType, typename OutType>
BootstrapCalculator<InType, OutType>::BootstrapCalculator(
    const libMesh::ParallelObject & other,
    const std::string & name,
    const std::vector<Real> & levels,
    unsigned int replicates,
    unsigned int seed,
    StochasticTools::Calculator<InType, OutType> & calc)
  : libMesh::ParallelObject(other),
    _levels(levels),
    _replicates(replicates),
    _seed(seed),
    _calc(calc),
    _name(name)
{
  mooseAssert(*std::min_element(levels.begin(), levels.end()) > 0,
              "The supplied levels must be greater than zero.");
  mooseAssert(*std::max_element(levels.begin(), levels.end()) < 1,
              "The supplied levels must be less than one");
}

template <typename InType, typename OutType>
std::vector<OutType>
BootstrapCalculator<InType, OutType>::computeBootstrapEstimates(const InType & data,
                                                                const bool is_distributed)
{
  MooseRandom generator;
  generator.seed(0, _seed);

  // Compute replicate statistics
  std::vector<OutType> values(_replicates);
  for (std::size_t i = 0; i < _replicates; ++i)
    values[i] = resample(_calc, data, generator, is_distributed);
  std::sort(values.begin(), values.end());
  return values;
}

template <typename InType, typename OutType>
OutType
BootstrapCalculator<InType, OutType>::resample(StochasticTools::Calculator<InType, OutType> & calc,
                                               const InType & data,
                                               MooseRandom & generator,
                                               const bool is_distributed)
{
  const std::size_t seed_index = 0;
  const std::size_t n_local = data.size();

  calc.initialize();

  if (!is_distributed)
  {
    for (std::size_t j = 0; j < n_local; ++j)
    {
      auto index = generator.randl(seed_index, 0, n_local);
      calc.update(data[index]);
    }
  }
  else
  {
    // Compute the global size of the vector
    std::size_t n_global = n_local;
    this->_communicator.sum(n_global);

    // Compute the vector data offsets, the scope cleans up the "n_local" vector
    std::vector<std::size_t> offsets(this->_communicator.size());
    {
      std::vector<std::size_t> local_sizes;
      this->_communicator.allgather(n_local, local_sizes);
      for (std::size_t i = 0; i < local_sizes.size() - 1; ++i)
        offsets[i + 1] = offsets[i] + local_sizes[i];
    }

    // Advance the random number generator to the current offset
    const auto rank = this->_communicator.rank();
    for (std::size_t i = 0; i < offsets[rank]; ++i)
      generator.randl(seed_index, 0, n_global);

    // Compute the needs for this processor
    std::unordered_map<processor_id_type, std::vector<std::size_t>> indices;
    for (std::size_t i = 0; i < n_local; ++i)
    {
      const auto idx = generator.randl(seed_index, 0, n_global); // random global index

      // Locate the rank and local index of the data desired
      const auto idx_offset_iter = std::prev(std::upper_bound(offsets.begin(), offsets.end(), idx));
      const auto idx_rank = std::distance(offsets.begin(), idx_offset_iter);
      const auto idx_local_idx = idx - *idx_offset_iter;

      // Push back the index to appropriate rank
      indices[idx_rank].push_back(idx_local_idx);
    }

    // Advance the random number generator to the end of the global vector
    for (std::size_t i = offsets[rank] + n_local; i < n_global; ++i)
      generator.randl(seed_index, 0, n_global);

    // Send the indices to the appropriate rank and have the calculator do its work
    auto calc_functor =
        [&data, &calc](processor_id_type /*pid*/,
                       const std::vector<std::size_t> & indices) {
          for (const auto & idx : indices)
            calc.update(data[idx]);
        };
    Parallel::push_parallel_vector_data(this->_communicator, indices, calc_functor);
  }

  calc.finalize(is_distributed);
  return calc.get();
}


// PERCENTILE //////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
std::vector<OutType>
Percentile<InType, OutType>::compute(const InType & data, const bool is_distributed)
{
  // Bootstrap estimates
  const std::vector<OutType> values = this->computeBootstrapEstimates(data, is_distributed);

  // Extract percentiles
  std::vector<OutType> output;
  if (this->processor_id() == 0)
    for (const Real & level : this->_levels)
    {
      long unsigned int index = std::lrint(level * (this->_replicates - 1));
      output.push_back(values[index]);
    }

  return output;
}

// BIASCORRECTEDACCELERATED ////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
std::vector<OutType>
BiasCorrectedAccelerated<InType, OutType>::compute(const InType & data,
                                                   const bool is_distributed)
{
  if (is_distributed)
    mooseError("Due to the computational demands, the BiasCorrectedAccelerated does not work with "
               "distributed data.");

  // Bootstrap estimates
  const std::vector<OutType> values = this->computeBootstrapEstimates(data, is_distributed);

  // Compute bias-correction, Efron and Tibshirani (2003), Eq. 14.14, p. 186
  const OutType value = this->_calc.compute(data, is_distributed);
  const Real count = std::count_if(values.begin(), values.end(), [&value](OutType v) {
    return v < value;
  }); // use Real for non-integer division below
  const Real bias = NormalDistribution::quantile(count / this->_replicates, 0, 1);

  // Compute Acceleration, Efron and Tibshirani (2003), Eq. 14.15, p. 186
  const Real acc = data.empty() ? 0. : acceleration(data, is_distributed);

  // Compute intervals, Efron and Tibshirani (2003), Eq. 14.10, p. 185
  std::vector<OutType> output;
  for (const Real & level : this->_levels)
  {
    const Real z = NormalDistribution::quantile(level, 0, 1);
    const Real x = bias + (bias + (bias + z) / (1 - acc * (bias + z)));
    const Real alpha = NormalDistribution::cdf(x, 0, 1);

    long unsigned int index = std::lrint(alpha * (this->_replicates - 1));
    output.push_back(values[index]);
  }
  return output;
}

template <typename InType, typename OutType>
Real
BiasCorrectedAccelerated<InType, OutType>::acceleration(const InType & data,
                                                        const bool is_distributed)
{
  // Jackknife statistics
  InType theta_i(data.size());

  // Total number of data entries
  Real count = data.size();

  // Compute jackknife estimates, Ch. 11, Eq. 11.2, p. 141
  InType data_not_i(data.size() - 1);
  for (std::size_t i = 0; i < count; ++i)
  {
    std::copy(data.begin(), data.begin() + i, data_not_i.begin());
    std::copy(data.begin() + i + 1, data.end(), data_not_i.begin() + i);
    theta_i[i] = this->_calc.compute(data_not_i, is_distributed);
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

// makeBootstrapCalculator /////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
std::unique_ptr<BootstrapCalculator<InType, OutType>>
makeBootstrapCalculator(const MooseEnum & item,
                        const libMesh::ParallelObject & other,
                        const std::vector<Real> & levels,
                        unsigned int replicates,
                        unsigned int seed,
                        StochasticTools::Calculator<InType, OutType> & calc)
{
  std::unique_ptr<BootstrapCalculator<InType, OutType>> ptr = nullptr;
  if (item == "percentile")
    ptr = libmesh_make_unique<Percentile<InType, OutType>>(
        other, item, levels, replicates, seed, calc);
  else if (item == "bca")
    ptr = libmesh_make_unique<BiasCorrectedAccelerated<InType, OutType>>(
        other, item, levels, replicates, seed, calc);

  if (!ptr)
    ::mooseError("Failed to create Statistics::BootstrapCalculator object for ", item);

  return ptr;
}

#define createBootstrapCalculators(InType, OutType)                                                \
  template class Percentile<InType, OutType>;                                                      \
  template class BiasCorrectedAccelerated<InType, OutType>;                                        \
  template std::unique_ptr<BootstrapCalculator<InType, OutType>>                                   \
  makeBootstrapCalculator<InType, OutType>(const MooseEnum &,                                      \
                                           const libMesh::ParallelObject &,                        \
                                           const std::vector<Real> &,                              \
                                           unsigned int,                                           \
                                           unsigned int,                                           \
                                           StochasticTools::Calculator<InType, OutType> &)

createBootstrapCalculators(std::vector<Real>, Real);
createBootstrapCalculators(std::vector<int>, Real);

} // namespace
