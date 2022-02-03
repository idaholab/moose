//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "Calculators.h"
#include "NormalDistribution.h"
#include "StochasticToolsUtils.h"

#include "Shuffle.h"
#include "MooseTypes.h"
#include "MooseObject.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseRandom.h"

#include "libmesh/parallel.h"
#include "libmesh/parallel_sync.h"

#include <memory>
#include <vector>

class MooseEnum;
class MooseEnumItem;
class MooseRandom;

namespace StochasticTools
{
template <typename InType, typename OutType>
class Calculator;

/*
 * Return available bootstrap statistics calculators.
 */
MooseEnum makeBootstrapCalculatorEnum();

/**
 * Base class for computing bootstrap confidence level intervals. These classes follow the same
 * design pattern as those Statistics.h.
 * @param other ParallelObject that is providing the Communicator
 * @param levels The bootstrap confidence level intervals to compute in range (0, 1)
 * @param replicates Number of bootstrap replicates to perform
 * @param seed Seed for random number generator
 */
template <typename InType, typename OutType>
class BootstrapCalculator : public libMesh::ParallelObject
{
public:
  BootstrapCalculator(const libMesh::ParallelObject & other,
                      const std::string & name,
                      const std::vector<Real> & levels,
                      unsigned int replicates,
                      unsigned int seed,
                      StochasticTools::Calculator<InType, OutType> & calc);
  virtual std::vector<OutType> compute(const InType &, const bool) = 0;
  const std::string & name() const { return _name; }

protected:
  // Compute Bootstrap estimates of a statistic
  std::vector<OutType> computeBootstrapEstimates(const InType &, const bool);

  // Confidence levels to compute in range (0, 1)
  const std::vector<Real> _levels;

  // Number of bootstrap replicates
  const unsigned int _replicates;

  // Random seed for creating bootstrap replicates
  const unsigned int _seed;

  // The Calculator that computes the statistic of interest
  StochasticTools::Calculator<InType, OutType> & _calc;

private:
  const std::string _name;
};

/*
 * Implement percentile method of Efron and Tibshirani (2003), Chapter 13.
 */
template <typename InType, typename OutType>
class Percentile : public BootstrapCalculator<InType, OutType>
{
public:
  using BootstrapCalculator<InType, OutType>::BootstrapCalculator;
  virtual std::vector<OutType> compute(const InType &, const bool) override;
};

/*
 * Implement BCa method of Efron and Tibshirani (2003), Chapter 14.
 */
template <typename InType, typename OutType>
class BiasCorrectedAccelerated : public BootstrapCalculator<InType, OutType>
{
public:
  using BootstrapCalculator<InType, OutType>::BootstrapCalculator;
  virtual std::vector<OutType> compute(const InType &, const bool) override;

private:
  // Compute the acceleration, see Efron and Tibshirani (2003), Ch. 14, Eq. 14.15, p 186.
  OutType acceleration(const InType &, const bool);
};

/*
 * Simple struct that makeBootstrapCalculator wraps around, this is so building calculators
 * can be partially specialized.
 */
template <typename InType, typename OutType>
struct BootstrapCalculatorBuilder
{
  static std::unique_ptr<BootstrapCalculator<InType, OutType>>
  build(const MooseEnum &,
        const libMesh::ParallelObject &,
        const std::vector<Real> &,
        unsigned int,
        unsigned int,
        StochasticTools::Calculator<InType, OutType> &);
};

template <typename InType, typename OutType>
std::unique_ptr<BootstrapCalculator<InType, OutType>>
makeBootstrapCalculator(const MooseEnum &,
                        const libMesh::ParallelObject &,
                        const std::vector<Real> &,
                        unsigned int,
                        unsigned int,
                        StochasticTools::Calculator<InType, OutType> &);

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
  auto calc_update = [this](const typename InType::value_type & val)
  { _calc.updateCalculator(val); };
  for (std::size_t i = 0; i < _replicates; ++i)
  {
    _calc.initializeCalculator();
    MooseUtils::resampleWithFunctor(
        data, calc_update, generator, 0, is_distributed ? &this->_communicator : nullptr);
    _calc.finalizeCalculator(is_distributed);
    values[i] = _calc.getValue();
  }
  inplaceSort(values);
  return values;
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
  return BootstrapCalculatorBuilder<InType, OutType>::build(
      item, other, levels, replicates, seed, calc);
}

} // namespace
