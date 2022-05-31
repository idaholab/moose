//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorCalculators.h"

namespace StochasticTools
{

template <typename InType, typename OutType>
std::vector<std::vector<OutType>>
Percentile<std::vector<InType>, std::vector<OutType>>::compute(const std::vector<InType> & data,
                                                               const bool is_distributed)
{
  // Bootstrap estimates
  const auto values = this->computeBootstrapEstimates(data, is_distributed);

  // Extract percentiles
  std::vector<std::vector<OutType>> output;
  if (this->processor_id() == 0)
    for (const Real & level : this->_levels)
    {
      long unsigned int index = std::lrint(level * (this->_replicates - 1));
      output.push_back(values[index]);
    }

  return output;
}

template <typename InType, typename OutType>
std::vector<std::vector<OutType>>
BiasCorrectedAccelerated<std::vector<InType>, std::vector<OutType>>::compute(
    const std::vector<InType> & data, bool is_distributed)
{
  // Bootstrap estimates
  const std::vector<std::vector<OutType>> values =
      this->computeBootstrapEstimates(data, is_distributed);

  // Compute bias-correction, Efron and Tibshirani (2003), Eq. 14.14, p. 186
  const std::vector<OutType> value = this->_calc.compute(data, is_distributed);
  std::vector<Real> bias(value.size());
  for (const auto & i : index_range(value))
  {
    Real count = 0.;
    for (const auto & val : values)
    {
      if (val[i] < value[i])
        count++;
      else
        break;
    }
    bias[i] = NormalDistribution::quantile(count / this->_replicates, 0, 1);
  }

  // Compute Acceleration, Efron and Tibshirani (2003), Eq. 14.15, p. 186
  const std::vector<OutType> acc =
      data.empty() ? std::vector<OutType>(value.size()) : acceleration(data, is_distributed);

  // Compute intervals, Efron and Tibshirani (2003), Eq. 14.10, p. 185
  std::vector<std::vector<OutType>> output(this->_levels.size(),
                                           std::vector<OutType>(value.size()));
  for (const auto & l : index_range(this->_levels))
  {
    const Real z = NormalDistribution::quantile(this->_levels[l], 0, 1);
    for (const auto & i : index_range(value))
    {
      const OutType x = bias[i] + (bias[i] + (bias[i] + z) / (1 - acc[i] * (bias[i] + z)));
      const Real alpha = NormalDistribution::cdf(x, 0, 1);

      long unsigned int index = std::lrint(alpha * (this->_replicates - 1));
      output[l][i] = values[index][i];
    }
  }
  return output;
}

template <typename InType, typename OutType>
std::vector<OutType>
BiasCorrectedAccelerated<std::vector<InType>, std::vector<OutType>>::acceleration(
    const std::vector<InType> & data, const bool is_distributed)
{
  const std::size_t local_size = data.size();
  std::vector<std::size_t> local_sizes = {local_size};
  if (is_distributed)
    this->_communicator.allgather(local_sizes);
  const std::size_t count = std::accumulate(local_sizes.begin(), local_sizes.end(), 0);
  const processor_id_type rank = is_distributed ? this->processor_id() : 0;

  // Jackknife statistics
  std::vector<std::vector<OutType>> theta_i(local_size);

  // Compute jackknife estimates, Ch. 11, Eq. 11.2, p. 141
  for (processor_id_type r = 0; r < local_sizes.size(); ++r)
    for (std::size_t i = 0; i < local_sizes[r]; ++i)
    {
      this->_calc.initializeCalculator();
      for (std::size_t il = 0; il < local_size; ++il)
        if (i != il || r != rank)
          this->_calc.updateCalculator(data[il]);
      this->_calc.finalizeCalculator(is_distributed);
      if (r == rank)
        theta_i[i] = this->_calc.getValue();
    }

  // Compute jackknife sum, Ch. 11, Eq. 11.4, p. 141
  const std::size_t nval = theta_i.size() > 0 ? theta_i[0].size() : 0;
  std::vector<OutType> theta_dot(nval, 0.);
  for (const auto & ti : theta_i)
    for (const auto & i : make_range(nval))
      theta_dot[i] += ti[i] / count;
  if (is_distributed)
    this->_communicator.sum(theta_dot);

  // Acceleration, Ch. 14, Eq. 14.15, p. 185
  std::vector<OutType> num_den(2 * nval);
  for (const auto & jk : theta_i)
    for (const auto & i : make_range(nval))
    {
      num_den[i] += MathUtils::pow(theta_dot[i] - jk[i], 3);
      num_den[nval + i] += MathUtils::pow(theta_dot[i] - jk[i], 2);
    }
  if (is_distributed)
    this->_communicator.sum(num_den);

  mooseAssert(std::find(num_den.begin() + nval, num_den.end(), OutType()) == num_den.end(),
              "The acceleration denomenator must not be zero.");
  std::vector<OutType> acc(nval);
  for (const auto & i : make_range(nval))
    acc[i] = num_den[i] / (6 * std::pow(num_den[nval + i], 3. / 2.));
  return acc;
}

template <typename InType, typename OutType>
std::unique_ptr<Calculator<std::vector<InType>, std::vector<OutType>>>
CalculatorBuilder<std::vector<InType>, std::vector<OutType>>::build(
    const MooseEnumItem & item, const libMesh::ParallelObject & other)
{
  if (item == "min")
    return std::make_unique<VectorCalculator<InType, OutType, Min>>(other, item);

  else if (item == "max")
    return std::make_unique<VectorCalculator<InType, OutType, Max>>(other, item);

  else if (item == "sum")
    return std::make_unique<VectorCalculator<InType, OutType, Sum>>(other, item);

  else if (item == "mean" || item == "average") // average is deprecated
    return std::make_unique<VectorCalculator<InType, OutType, Mean>>(other, item);

  else if (item == "stddev")
    return std::make_unique<VectorCalculator<InType, OutType, StdDev>>(other, item);

  else if (item == "stderr")
    return std::make_unique<VectorCalculator<InType, OutType, StdErr>>(other, item);

  else if (item == "norm2")
    return std::make_unique<VectorCalculator<InType, OutType, L2Norm>>(other, item);

  else if (item == "ratio")
    return std::make_unique<VectorCalculator<InType, OutType, Ratio>>(other, item);

  else if (item == "median")
    return std::make_unique<VectorCalculator<InType, OutType, Median>>(other, item);

  else if (item == "meanabs")
    return std::make_unique<VectorCalculator<InType, OutType, MeanAbsoluteValue>>(other, item);

  ::mooseError("Failed to create Statistics::Calculator object for ", item);
  return nullptr;
}

template <typename InType, typename OutType>
std::unique_ptr<BootstrapCalculator<std::vector<InType>, std::vector<OutType>>>
BootstrapCalculatorBuilder<std::vector<InType>, std::vector<OutType>>::build(
    const MooseEnum & item,
    const libMesh::ParallelObject & other,
    const std::vector<Real> & levels,
    unsigned int replicates,
    unsigned int seed,
    StochasticTools::Calculator<std::vector<InType>, std::vector<OutType>> & calc)
{
  std::unique_ptr<BootstrapCalculator<std::vector<InType>, std::vector<OutType>>> ptr = nullptr;
  if (item == "percentile")
    ptr = std::make_unique<Percentile<std::vector<InType>, std::vector<OutType>>>(
        other, item, levels, replicates, seed, calc);
  else if (item == "bca")
    ptr = std::make_unique<BiasCorrectedAccelerated<std::vector<InType>, std::vector<OutType>>>(
        other, item, levels, replicates, seed, calc);
  else
    ::mooseError("Failed to create Statistics::BootstrapCalculator object for ", item);

  return ptr;
}

#define createVectorCalculators(InType, OutType)                                                   \
  template class Percentile<std::vector<InType>, std::vector<OutType>>;                            \
  template class BiasCorrectedAccelerated<std::vector<InType>, std::vector<OutType>>;              \
  template struct CalculatorBuilder<std::vector<InType>, std::vector<OutType>>;                    \
  template struct BootstrapCalculatorBuilder<std::vector<InType>, std::vector<OutType>>

createVectorCalculators(std::vector<Real>, Real);

} // StocasticTools namespace
