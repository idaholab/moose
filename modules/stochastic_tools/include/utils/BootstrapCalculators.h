//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseTypes.h"
#include "MooseObject.h"
#include <vector>

class MooseEnum;
class MooseEnumItem;
class MooseRandom;

namespace StochasticTools
{
class Calculator;
class BootstrapCalculator;

/*
 * Return available bootstrap statistics calculators.
 */
MooseEnum makeBootstrapCalculatorEnum();

/*
 * Create const Bootstrap confidence level interface calculator for use by VectorPostprocessor
 * objects.
 */
std::unique_ptr<const BootstrapCalculator> makeBootstrapCalculator(const MooseEnum &,
                                                                   const libMesh::ParallelObject &,
                                                                   const std::vector<Real> &,
                                                                   unsigned int,
                                                                   unsigned int);

/**
 * Base class for computing bootstrap confidence level intervals. These classes follow the same
 * design pattern as those Statistics.h.
 * @param other ParallelObject that is providing the Communicator
 * @param levels The bootstrap confidence level intervals to compute in range (0, 1)
 * @param replicates Number of bootstrap replicates to perform
 * @param seed Seed for random number generator
 */
class BootstrapCalculator : public libMesh::ParallelObject
{
public:
  BootstrapCalculator(const libMesh::ParallelObject & other,
                      const std::vector<Real> & levels,
                      unsigned int replicates,
                      unsigned int seed);
  virtual ~BootstrapCalculator() = default;

  /**
   * Compute the bootstrap confidence level intervals.
   * @param data Vector of data from which statistics are to be computed
   * @param calc Calculator object defining the statistic to be computed
   * @param is_distributed Flag indicating if the data is distributed in parallel
   */
  virtual std::vector<Real> compute(const std::vector<Real> & data,
                                    const Calculator & calc,
                                    const bool is_distributed) const = 0;

protected:
  // Compute Bootstrap estimates of a statistic
  std::vector<Real>
  computeBootstrapEstimates(const std::vector<Real> &, const Calculator &, const bool) const;

  // Randomly shuffle a vector of data
  std::vector<Real> shuffle(const std::vector<Real> &, MooseRandom &, const bool) const;

  // Confidence levels to compute in range (0, 1)
  const std::vector<Real> _levels;

  // Number of bootstrap replicates
  const unsigned int _replicates;

  // Random seed for creating bootstrap replicates
  const unsigned int _seed;
};

/*
 * Implement percentile method of Efron and Tibshirani (2003), Chapter 13.
 */
class Percentile : public BootstrapCalculator
{
public:
  Percentile(const libMesh::ParallelObject & other,
             const std::vector<Real> & levels,
             unsigned int replicates,
             unsigned int seed);

  virtual std::vector<Real>
  compute(const std::vector<Real> &, const Calculator &, const bool) const override;
};

/*
 * Implement BCa method of Efron and Tibshirani (2003), Chapter 14.
 */
class BiasCorrectedAccelerated : public BootstrapCalculator
{
public:
  BiasCorrectedAccelerated(const libMesh::ParallelObject & other,
                           const std::vector<Real> & levels,
                           unsigned int replicates,
                           unsigned int seed);

  virtual std::vector<Real>
  compute(const std::vector<Real> &, const Calculator &, const bool) const override;

private:
  // Compute the acceleration, see Efron and Tibshirani (2003), Ch. 14, Eq. 14.15, p 186.
  Real acceleration(const std::vector<Real> &, const Calculator &, const bool) const;
};

} // namespace
