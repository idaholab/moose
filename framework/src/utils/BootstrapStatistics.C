//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
// Bootstrap CI

#include "Statistics.h"
#include "BootstrapStatistics.h"
#include "MooseEnumItem.h"
#include "MooseEnum.h"
#include "MooseError.h"
#include "MooseRandom.h"
#include "libmesh/auto_ptr.h"
#include "libmesh/parallel.h"

namespace Statistics
{

MooseEnum
makeBootstrapCalculatorEnum()
{
  return MooseEnum("percentile=0");
}

std::unique_ptr<const BootstrapCalculator>
makeBootstrapCalculator(const MooseEnum & item,
                        const MooseObject & other,
                        const std::vector<Real> & levels,
                        unsigned int replicates,
                        unsigned int seed)
{

  std::unique_ptr<BootstrapCalculator> ptr = nullptr;
  if (item == "percentile")
    ptr = libmesh_make_unique<Percentile>(other);

  ptr->setLevels(levels);
  ptr->setReplicates(replicates);
  ptr->setSeed(seed);
  if (!ptr)
    ::mooseError("Failed to create Statistics::BootstrapCalculator object for ", item);
  return ptr;
}

BootstrapCalculator::BootstrapCalculator(const MooseObject & other) : libMesh::ParallelObject(other)
{
}

void
BootstrapCalculator::setLevels(std::vector<Real> levels)
{
  mooseAssert(*std::min_element(levels.begin(), levels.end()) > 0,
              "The supplied levels must be greater than zero.");
  mooseAssert(*std::max_element(levels.begin(), levels.end()) < 1,
              "The supplied levels must be less than one");
  _levels = levels;
}

void
BootstrapCalculator::setReplicates(const unsigned int replicates)
{
  _replicates = replicates;
}

void
BootstrapCalculator::setSeed(const unsigned int seed)
{
  _seed = seed;
}

std::vector<Real>
BootstrapCalculator::shuffle(const std::vector<Real> & data,
                             MooseRandom & generator,
                             const bool is_distributed) const
{
  const std::size_t n = data.size();
  std::vector<Real> replicate(n);
  if (!is_distributed)
  {
    for (std::size_t j = 0; j < n; ++j)
    {
      auto index = generator.randl(0, 0, n);
      replicate[j] = data[index];
    }
  }

  else
  {
    // from future import parallel magic
  }

  return replicate;
}

// PERCENTILE //////////////////////////////////////////////////////////////////////////////////////
Percentile::Percentile(const MooseObject & other) : BootstrapCalculator(other) {}

std::vector<Real>
Percentile::compute(const std::vector<Real> & data,
                    const Calculator & calc,
                    const bool is_distributed) const
{
  MooseRandom generator;
  generator.seed(0, _seed);

  // Compute replicate statistics
  std::vector<Real> values(_replicates);
  for (std::size_t i = 0; i < _replicates; ++i)
  {
    std::vector<Real> replicate = shuffle(data, generator, is_distributed);
    values[i] = calc.compute(replicate, is_distributed);
  }
  std::sort(values.begin(), values.end());

  // Capture replicates
  std::vector<Real> output;
  for (const Real & level : _levels)
  {
    long unsigned int index = std::lrint(level * (_replicates - 1));
    output.push_back(values[index]);
  }

  return output;
}

} // namespace
