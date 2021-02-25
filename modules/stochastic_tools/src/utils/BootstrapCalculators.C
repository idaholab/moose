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
    const StochasticTools::Calculator<InType, OutType> & calc)
  : Calculator<InType, std::vector<OutType>>(other, name),
    _levels(levels),
    _replicates(replicates),
    _seed(seed),
    _calc(calc)
{
  mooseAssert(*std::min_element(levels.begin(), levels.end()) > 0,
              "The supplied levels must be greater than zero.");
  mooseAssert(*std::max_element(levels.begin(), levels.end()) < 1,
              "The supplied levels must be less than one");
}

template <typename InType, typename OutType>
std::vector<OutType>
BootstrapCalculator<InType, OutType>::computeBootstrapEstimates(const InType & data,
                                                                const bool is_distributed) const
{
  MooseRandom generator;
  generator.seed(0, _seed);

  // Compute replicate statistics
  std::vector<OutType> values(_replicates);
  for (std::size_t i = 0; i < _replicates; ++i)
  {
    InType replicate = resample(data, generator, is_distributed);
    values[i] = _calc.compute(replicate, is_distributed);
  }
  std::sort(values.begin(), values.end());
  return values;
}

template <typename InType, typename OutType>
InType
BootstrapCalculator<InType, OutType>::resample(const InType & data,
                                               MooseRandom & generator,
                                               const bool is_distributed) const
{
  return MooseUtils::resample(data, generator, 0, is_distributed ? &this->_communicator : nullptr);
}

// PERCENTILE //////////////////////////////////////////////////////////////////////////////////////
template <typename InType, typename OutType>
std::vector<OutType>
Percentile<InType, OutType>::compute(const InType & data, const bool is_distributed) const
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
                                                   const bool is_distributed) const
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
                                                        const bool is_distributed) const
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
std::unique_ptr<const BootstrapCalculator<InType, OutType>>
makeBootstrapCalculator(const MooseEnum & item,
                        const libMesh::ParallelObject & other,
                        const std::vector<Real> & levels,
                        unsigned int replicates,
                        unsigned int seed,
                        const StochasticTools::Calculator<InType, OutType> & calc)
{
  std::unique_ptr<const BootstrapCalculator<InType, OutType>> ptr = nullptr;
  if (item == "percentile")
    ptr = libmesh_make_unique<const Percentile<InType, OutType>>(
        other, item, levels, replicates, seed, calc);
  else if (item == "bca")
    ptr = libmesh_make_unique<const BiasCorrectedAccelerated<InType, OutType>>(
        other, item, levels, replicates, seed, calc);

  if (!ptr)
    ::mooseError("Failed to create Statistics::BootstrapCalculator object for ", item);

  return ptr;
}

#define createBootstrapCalculators(InType, OutType)                                                \
  template class Percentile<InType, OutType>;                                                      \
  template class BiasCorrectedAccelerated<InType, OutType>;                                        \
  template std::unique_ptr<const BootstrapCalculator<InType, OutType>>                             \
  makeBootstrapCalculator<InType, OutType>(const MooseEnum &,                                      \
                                           const libMesh::ParallelObject &,                        \
                                           const std::vector<Real> &,                              \
                                           unsigned int,                                           \
                                           unsigned int,                                           \
                                           const StochasticTools::Calculator<InType, OutType> &)

createBootstrapCalculators(std::vector<Real>, Real);
createBootstrapCalculators(std::vector<int>, Real);

} // namespace
