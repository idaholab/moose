//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
// Bootstrap CI

#include "BootstrapCalculators.h"

namespace StochasticTools
{

MooseEnum
makeBootstrapCalculatorEnum()
{
  return MooseEnum("percentile=0 bca=1");
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
BiasCorrectedAccelerated<InType, OutType>::compute(const InType & data, const bool is_distributed)
{
  if (is_distributed)
    mooseError("Due to the computational demands, the BiasCorrectedAccelerated does not work with "
               "distributed data.");

  // Bootstrap estimates
  const std::vector<OutType> values = this->computeBootstrapEstimates(data, is_distributed);

  // Compute bias-correction, Efron and Tibshirani (2003), Eq. 14.14, p. 186
  const OutType value = this->_calc.compute(data, is_distributed);
  const Real count = std::count_if(values.begin(), values.end(), [&value](Real v) {
    return v < value;
  }); // use Real for non-integer division below
  const Real bias = NormalDistribution::quantile(count / this->_replicates, 0, 1);

  // Compute Acceleration, Efron and Tibshirani (2003), Eq. 14.15, p. 186
  const OutType acc = data.empty() ? OutType() : acceleration(data, is_distributed);

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
OutType
BiasCorrectedAccelerated<InType, OutType>::acceleration(const InType & data,
                                                        const bool is_distributed)
{
  // Jackknife statistics
  std::vector<OutType> theta_i(data.size());

  // Total number of data entries
  const std::size_t count = data.size();

  // Compute jackknife estimates, Ch. 11, Eq. 11.2, p. 141
  InType data_not_i(data.size() - 1);
  for (std::size_t i = 0; i < count; ++i)
  {
    std::copy(data.begin(), data.begin() + i, data_not_i.begin());
    std::copy(data.begin() + i + 1, data.end(), data_not_i.begin() + i);
    theta_i[i] = this->_calc.compute(data_not_i, is_distributed);
  }

  // Compute jackknife sum, Ch. 11, Eq. 11.4, p. 141
  OutType theta_dot = std::accumulate(theta_i.begin(), theta_i.end(), OutType());
  theta_dot /= count;

  // Acceleration, Ch. 14, Eq. 14.15, p. 185
  OutType numerator = OutType();
  OutType denomenator = OutType();
  for (const auto & jk : theta_i)
  {
    numerator += MathUtils::pow(theta_dot - jk, 3);
    denomenator += MathUtils::pow(theta_dot - jk, 2);
  }

  mooseAssert(denomenator != OutType(), "The acceleration denomenator must not be zero.");
  return numerator / (6 * std::pow(denomenator, 3. / 2.));
}

template <typename InType, typename OutType>
std::unique_ptr<BootstrapCalculator<InType, OutType>>
BootstrapCalculatorBuilder<InType, OutType>::build(
    const MooseEnum & item,
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
  template struct BootstrapCalculatorBuilder<InType, OutType>

createBootstrapCalculators(std::vector<Real>, Real);
createBootstrapCalculators(std::vector<int>, Real);

} // namespace
