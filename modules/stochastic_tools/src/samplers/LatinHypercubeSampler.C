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
  : Sampler(parameters),
    _shufflers(declareRestartableData<std::vector<std::unique_ptr<MooseRandomPerturbation>>>("lhs_shufflers"))
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
LatinHypercubeSampler::executeTearDown()
{
  _shufflers.clear();
  for (const auto col : make_range(getNumberOfCols()))
  {
    const auto seed = getRandl(col, 0, std::numeric_limits<uint32_t>::max(), 1);
    _shufflers.push_back(std::make_unique<MooseRandomPerturbation>(seed, getNumberOfRows()));
  }
}

Real
LatinHypercubeSampler::computeSample(dof_id_type row_index, dof_id_type col_index) const
{
  mooseAssert(_shufflers.size() > 0, "Shufflers have not been initialized.");

  // Divide [0,1] into N equal bins of width 1/N.
  const Real bin_size = 1. / getNumberOfRows();

  // Map row_index to a shuffled bin via the column's bijective permutation.
  // Because permute() is a bijection on [0, N), each row gets a distinct bin,
  // which is the core LHS stratification guarantee.
  const auto bin = _shufflers[col_index]->permute(row_index);

  // Draw a uniform random point within the selected bin.
  const auto lower = bin * bin_size;
  const auto upper = (bin + 1) * bin_size;
  const Real probability =
      getRand(row_index * getNumberOfCols() + col_index) * (upper - lower) + lower;

  // Transform the probability through the inverse CDF to obtain the sample value.
  return _distributions[col_index]->quantile(probability);
}
