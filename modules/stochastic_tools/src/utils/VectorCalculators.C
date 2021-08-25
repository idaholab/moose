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
std::unique_ptr<Calculator<std::vector<InType>, std::vector<OutType>>>
CalculatorBuilder<std::vector<InType>, std::vector<OutType>>::build(
    const MooseEnumItem & item, const libMesh::ParallelObject & other)
{
  if (item == "min")
    return libmesh_make_unique<VectorCalculator<InType, OutType, Min>>(other, item);

  else if (item == "max")
    return libmesh_make_unique<VectorCalculator<InType, OutType, Max>>(other, item);

  else if (item == "sum")
    return libmesh_make_unique<VectorCalculator<InType, OutType, Sum>>(other, item);

  else if (item == "mean" || item == "average") // average is deprecated
    return libmesh_make_unique<VectorCalculator<InType, OutType, Mean>>(other, item);

  else if (item == "stddev")
    return libmesh_make_unique<VectorCalculator<InType, OutType, StdDev>>(other, item);

  else if (item == "stderr")
    return libmesh_make_unique<VectorCalculator<InType, OutType, StdErr>>(other, item);

  else if (item == "norm2")
    return libmesh_make_unique<VectorCalculator<InType, OutType, L2Norm>>(other, item);

  else if (item == "ratio")
    return libmesh_make_unique<VectorCalculator<InType, OutType, Ratio>>(other, item);

  else if (item == "median")
    return libmesh_make_unique<VectorCalculator<InType, OutType, Median>>(other, item);

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
    ptr = libmesh_make_unique<Percentile<std::vector<InType>, std::vector<OutType>>>(
        other, item, levels, replicates, seed, calc);
  else if (item == "bca")
    ::mooseError("BiasCorrectedAccelerated bootstrap calculator has not been implemented for "
                 "vector-type quantities.");
  else
    ::mooseError("Failed to create Statistics::BootstrapCalculator object for ", item);

  return ptr;
}

#define createVectorCalculators(InType, OutType)                                                   \
  template class Percentile<std::vector<InType>, std::vector<OutType>>;                            \
  template struct CalculatorBuilder<std::vector<InType>, std::vector<OutType>>;                    \
  template struct BootstrapCalculatorBuilder<std::vector<InType>, std::vector<OutType>>

createVectorCalculators(std::vector<Real>, Real);

} // StocasticTools namespace
