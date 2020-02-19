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

namespace Statistics
{
/*
 * Base class for computing bootstrap confidence level intervals. These classes follow the same
 * design pattern as those Statistics.h.
 */
class BootstrapCalculator : public libMesh::ParallelObject
{
public:
  BootstrapCalculator(const libMesh::ParallelObject &,
                      const std::vector<Real> &,
                      unsigned int,
                      unsigned int);
  virtual ~BootstrapCalculator() = default;

  virtual std::vector<Real>
  compute(const std::vector<Real> &, const Calculator &, const bool) const = 0;

protected:
  std::vector<Real> shuffle(const std::vector<Real> &, MooseRandom &, const bool) const;

  // Confidence levels to compute in range (0, 1)
  const std::vector<Real> _levels;

  // Number of bootstrap replicates
  const unsigned int _replicates;

  // Random seed for creating boostrap replicates
  const unsigned int _seed;
};

/*
 * Implement percentile method of Efron and Tibshirani (2003), Chapter 13.
 */
class Percentile : public BootstrapCalculator
{
public:
  Percentile(const libMesh::ParallelObject &,
             const std::vector<Real> &,
             unsigned int,
             unsigned int);

  virtual std::vector<Real>
  compute(const std::vector<Real> &, const Calculator &, const bool) const override;
};

} // namespace Statistics
