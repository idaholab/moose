//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "VectorCalculators.h"

namespace StochasticTools
{

template <typename T>
using VecOfVec = std::vector<std::vector<T>>;

template <typename InType, typename OutType>
class Percentile<VecOfVec<InType>, VecOfVec<OutType>>
  : public BootstrapCalculator<VecOfVec<InType>, VecOfVec<OutType>>
{
public:
  using BootstrapCalculator<VecOfVec<InType>, VecOfVec<OutType>>::BootstrapCalculator;
  virtual std::vector<VecOfVec<OutType>> compute(const VecOfVec<InType> &, const bool) override;
};

template <typename InType, typename OutType>
struct BootstrapCalculatorBuilder<VecOfVec<InType>, VecOfVec<OutType>>
{
  static std::unique_ptr<BootstrapCalculator<VecOfVec<InType>, VecOfVec<OutType>>>
  build(const MooseEnum &,
        const libMesh::ParallelObject &,
        const std::vector<Real> &,
        unsigned int,
        unsigned int,
        StochasticTools::Calculator<VecOfVec<InType>, VecOfVec<OutType>> &);
};
}
