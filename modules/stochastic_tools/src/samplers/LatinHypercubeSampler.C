//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LatinHypercubeSampler.h"
#include "Distribution.h"
#include "MooseRandomPerturbation.h"

registerMooseObjectAliased("StochasticToolsApp", LatinHypercubeSampler, "LatinHypercube");

InputParameters
LatinHypercubeSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Latin Hypercube Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The size of the square matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  return params;
}

LatinHypercubeSampler::LatinHypercubeSampler(const InputParameters & parameters)
  : Sampler(parameters)
{
  const auto & distribution_names = getParam<std::vector<DistributionName>>("distributions");
  for (const DistributionName & name : distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(distribution_names.size());
  // Generator 0: within-bin uniform draws. Generator 1: column shuffler seeds.
  setNumberOfRandomSeeds(2);
}

void
LatinHypercubeSampler::sampleSetUp(const SampleMode mode)
{
  const bool is_global = mode == Sampler::SampleMode::GLOBAL;

  // Get seeds to use for shuffler construction
  std::vector<uint32_t> seeds(getNumberOfCols());
  if (is_global || processor_id() == 0)
  {
    for (dof_id_type col = 0; col < getNumberOfCols(); ++col)
      seeds[col] = getRandl(1, 0, std::numeric_limits<uint32_t>::max());
    // Need to restore generator to be consistent when sampler is called multiple times
    restoreGeneratorState();
  }
  if (!is_global)
    _local_comm.broadcast(seeds);

  _shufflers.clear();
  for (const auto & seed : seeds)
    _shufflers.push_back(std::make_unique<MooseRandomPerturbation>(seed, getNumberOfRows()));
}

Real
LatinHypercubeSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  // Divide [0,1] into N equal bins of width 1/N.
  const Real bin_size = 1. / getNumberOfRows();

  // Map row_index to a shuffled bin via the column's bijective permutation.
  // Because permute() is a bijection on [0, N), each row gets a distinct bin,
  // which is the core LHS stratification guarantee.
  const auto bin = _shufflers[col_index]->permute(row_index);

  // Draw a uniform random point within the selected bin.
  const auto lower = bin * bin_size;
  const auto upper = (bin + 1) * bin_size;
  const Real probability = getRand() * (upper - lower) + lower;

  // Need to increment index to be consistent when sampler is called multiple times
  getRand(1);

  // Transform the probability through the inverse CDF to obtain the sample value.
  return _distributions[col_index]->quantile(probability);
}
