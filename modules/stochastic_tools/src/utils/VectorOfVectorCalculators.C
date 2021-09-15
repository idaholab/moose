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
std::vector<VecOfVec<OutType>>
Percentile<VecOfVec<InType>, VecOfVec<OutType>>::compute(const VecOfVec<InType> & data,
                                                         const bool is_distributed)
{
  // Bootstrap estimates
  const auto values = this->computeBootstrapEstimates(data, is_distributed);

  // Extract percentiles
  std::vector<VecOfVec<OutType>> output;
  if (this->processor_id() == 0)
    for (const Real & level : this->_levels)
    {
      long unsigned int index = std::lrint(level * (this->_replicates - 1));
      output.push_back(values[index]);
    }

  return output;
}

template <typename InType, typename OutType>
std::unique_ptr<BootstrapCalculator<VecOfVec<InType>, VecOfVec<OutType>>>
BootstrapCalculatorBuilder<VecOfVec<InType>, VecOfVec<OutType>>::build(
    const MooseEnum & item,
    const libMesh::ParallelObject & other,
    const std::vector<Real> & levels,
    unsigned int replicates,
    unsigned int seed,
    StochasticTools::Calculator<VecOfVec<InType>, VecOfVec<OutType>> & calc)
{
  std::unique_ptr<BootstrapCalculator<VecOfVec<InType>, VecOfVec<OutType>>> ptr = nullptr;
  if (item == "percentile")
    ptr = libmesh_make_unique<Percentile<VecOfVec<InType>, VecOfVec<OutType>>>(
        other, item, levels, replicates, seed, calc);
  else
    ::mooseError("Failed to create Statistics::BootstrapCalculator object for ", item);

  return ptr;
}

#define createVectorOfVectorCalculators(InType, OutType)                                           \
  template class Percentile<VecOfVec<InType>, VecOfVec<OutType>>;                                  \
  template struct BootstrapCalculatorBuilder<VecOfVec<InType>, VecOfVec<OutType>>

createVectorOfVectorCalculators(std::vector<Real>, Real);
}
