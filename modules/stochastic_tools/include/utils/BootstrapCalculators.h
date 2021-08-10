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

  // Randomly shuffle a vector of data and apply calculator
  OutType resample(StochasticTools::Calculator<InType, OutType> &, const InType &, MooseRandom &, const bool);

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
 * This is placeholder class for BCa with general data types. Will throw an error if
 * used. Actual implementation is with OutType = Real which can be seen below.
 */
template <typename InType, typename OutType>
class BiasCorrectedAccelerated : public BootstrapCalculator<InType, OutType>
{
public:
  using BootstrapCalculator<InType, OutType>::BootstrapCalculator;
  virtual std::vector<OutType> compute(const InType &, const bool) override
  {
    mooseError(
        "Cannot compute bias corrected accelerated statistics with calculator output value type ",
        MooseUtils::prettyCppType<OutType>(),
        ".");
  }
};

/*
 * Implement BCa method of Efron and Tibshirani (2003), Chapter 14.
 */
template <typename InType>
class BiasCorrectedAccelerated<InType, Real> : public BootstrapCalculator<InType, Real>
{
public:
  using BootstrapCalculator<InType, Real>::BootstrapCalculator;
  virtual std::vector<Real> compute(const InType &, const bool) override;

private:
  // Compute the acceleration, see Efron and Tibshirani (2003), Ch. 14, Eq. 14.15, p 186.
  Real acceleration(const InType &, const bool);
};

template <typename InType, typename OutType>
std::unique_ptr<BootstrapCalculator<InType, OutType>>
makeBootstrapCalculator(const MooseEnum &,
                        const libMesh::ParallelObject &,
                        const std::vector<Real> &,
                        unsigned int,
                        unsigned int,
                        StochasticTools::Calculator<InType, OutType> & calc);

#define createBootstrapCalculators(InType, OutType)                                                \
  template class Percentile<InType, OutType>;                                                      \
  template class BiasCorrectedAccelerated<InType, OutType>;                                        \
  template std::unique_ptr<BootstrapCalculator<InType, OutType>>                                   \
  makeBootstrapCalculator<InType, OutType>(const MooseEnum &,                                      \
                                           const libMesh::ParallelObject &,                        \
                                           const std::vector<Real> &,                              \
                                           unsigned int,                                           \
                                           unsigned int,                                           \
                                           StochasticTools::Calculator<InType, OutType> &)
} // namespace
