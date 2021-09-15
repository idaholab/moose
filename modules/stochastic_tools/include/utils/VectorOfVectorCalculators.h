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

template <typename InType, typename OutType>
class Percentile<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>
  : public BootstrapCalculator<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>
{
public:
  using BootstrapCalculator<std::vector<std::vector<InType>>,
                            std::vector<std::vector<OutType>>>::BootstrapCalculator;
  virtual std::vector<std::vector<std::vector<OutType>>>
  compute(const std::vector<std::vector<InType>> &, const bool) override;
};

template <typename InType, typename OutType>
struct BootstrapCalculatorBuilder<std::vector<std::vector<InType>>,
                                  std::vector<std::vector<OutType>>>
{
  static std::unique_ptr<
      BootstrapCalculator<std::vector<std::vector<InType>>, std::vector<std::vector<OutType>>>>
  build(const MooseEnum &,
        const libMesh::ParallelObject &,
        const std::vector<Real> &,
        unsigned int,
        unsigned int,
        StochasticTools::Calculator<std::vector<std::vector<InType>>,
                                    std::vector<std::vector<OutType>>> &);
};
}
