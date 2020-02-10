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

/*
 * Base class for computing bootstrap confidence level intervals. These classes follow the same
 * design pattern as those Statistics.h.
 */
class BootstrapCalculator : public libMesh::ParallelObject
{
public:
  BootstrapCalculator(const libMesh::ParallelObject &);
  virtual ~BootstrapCalculator() = default;
  void setSeed(unsigned int);
  void setReplicates(unsigned int);
  void setLevels(std::vector<Real>);

  virtual std::vector<Real>
  compute(const std::vector<Real> &, const Calculator &, const bool) const = 0;

protected:
  std::vector<Real> shuffle(const std::vector<Real> &, MooseRandom &, const bool) const;
  std::vector<Real> _levels;
  unsigned int _seed = 1;
  unsigned int _replicates = 1000;
};

class Percentile : public BootstrapCalculator
{
public:
  Percentile(const libMesh::ParallelObject &);
  virtual std::vector<Real>
  compute(const std::vector<Real> &, const Calculator &, const bool) const override;
};

} // namespace Statistics
