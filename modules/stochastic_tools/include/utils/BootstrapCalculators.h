//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once
#include "Shuffle.h"
#include "MooseTypes.h"
#include "MooseObject.h"
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
class BootstrapCalculator : public Calculator<InType, std::vector<OutType>>
{
public:
  BootstrapCalculator(const libMesh::ParallelObject & other,
                      const std::string & name,
                      const std::vector<Real> & levels,
                      unsigned int replicates,
                      unsigned int seed,
                      const StochasticTools::Calculator<InType, OutType> & calc);

protected:
  // Compute Bootstrap estimates of a statistic
  std::vector<OutType> computeBootstrapEstimates(const InType &, const bool) const;

  // Randomly shuffle a vector of data
  InType resample(const InType &, MooseRandom &, const bool) const;

  // Confidence levels to compute in range (0, 1)
  const std::vector<Real> _levels;

  // Number of bootstrap replicates
  const unsigned int _replicates;

  // Random seed for creating bootstrap replicates
  const unsigned int _seed;

  // The Calculator that computes the statistic of interest
  const StochasticTools::Calculator<InType, OutType> & _calc;
};

/*
 * Implement percentile method of Efron and Tibshirani (2003), Chapter 13.
 */
template <typename InType, typename OutType>
class Percentile : public BootstrapCalculator<InType, OutType>
{
public:
  using BootstrapCalculator<InType, OutType>::BootstrapCalculator;
  virtual std::vector<OutType> compute(const InType &, const bool) const override;
};

/*
 * Implement BCa method of Efron and Tibshirani (2003), Chapter 14.
 */
template <typename InType, typename OutType>
class BiasCorrectedAccelerated : public BootstrapCalculator<InType, OutType>
{
public:
  using BootstrapCalculator<InType, OutType>::BootstrapCalculator;
  virtual std::vector<OutType> compute(const InType &, const bool) const override;

private:
  // Compute the acceleration, see Efron and Tibshirani (2003), Ch. 14, Eq. 14.15, p 186.
  Real acceleration(const InType &, const bool) const;
};

template <typename InType, typename OutType>
std::unique_ptr<const BootstrapCalculator<InType, OutType>>
makeBootstrapCalculator(const MooseEnum &,
                        const libMesh::ParallelObject &,
                        const std::vector<Real> &,
                        unsigned int,
                        unsigned int,
                        const StochasticTools::Calculator<InType, OutType> & calc);
} // namespace
