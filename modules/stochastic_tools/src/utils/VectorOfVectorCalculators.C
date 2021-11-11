//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorOfVectorCalculators.h"

namespace StochasticTools
{

template <typename InType, typename OutType>
std::vector<std::vector<std::vector<OutType>>>
Percentile<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>::compute(
    const std::vector<std::vector<InType>> & data, const bool is_distributed)
{
  // Bootstrap estimates
  const auto values = this->computeBootstrapEstimates(data, is_distributed);

  // Extract percentiles
  std::vector<std::vector<std::vector<OutType>>> output;
  if (this->processor_id() == 0)
    for (const Real & level : this->_levels)
    {
      long unsigned int index = std::lrint(level * (this->_replicates - 1));
      output.push_back(values[index]);
    }

  return output;
}

template <typename InType, typename OutType>
std::unique_ptr<
    BootstrapCalculator<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>>
BootstrapCalculatorBuilder<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>::
    build(const MooseEnum & item,
          const libMesh::ParallelObject & other,
          const std::vector<Real> & levels,
          unsigned int replicates,
          unsigned int seed,
          StochasticTools::Calculator<std::vector<std::vector<InType>>,
                                      std::vector<std::vector<OutType>>> & calc)
{
  std::unique_ptr<
      BootstrapCalculator<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>>
      ptr = nullptr;
  if (item == "percentile")
    ptr = std::make_unique<
        Percentile<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>>(
        other, item, levels, replicates, seed, calc);
  else
    ::mooseError("Failed to create Statistics::BootstrapCalculator object for ", item);

  return ptr;
}

#define createVectorOfVectorCalculators(InType, OutType)                                           \
  template class Percentile<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>;  \
  template struct BootstrapCalculatorBuilder<std::vector<std::vector<InType>>,                     \
                                             std::vector<std::vector<OutType>>>

createVectorOfVectorCalculators(std::vector<Real>, Real);
}
