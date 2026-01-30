//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NestedMonteCarloSampler.h"
#include "Distribution.h"

registerMooseObjectAliased("StochasticToolsApp", NestedMonteCarloSampler, "NestedMonteCarlo");

InputParameters
NestedMonteCarloSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Monte Carlo sampler for nested loops of parameters.");
  params.addRequiredParam<std::vector<dof_id_type>>(
      "num_rows",
      "The number of rows for each loop of parameters. The first number represents the outermost "
      "loop.");
  params.addRequiredParam<std::vector<std::vector<DistributionName>>>(
      "distributions",
      "Sets of distribution names to be sampled. Each set defines the parameters for the nested "
      "loop, with the first set being the outermost loop.");
  return params;
}

NestedMonteCarloSampler::NestedMonteCarloSampler(const InputParameters & parameters)
  : Sampler(parameters)
{
  // Grab inputs and make sure size is consistent
  const auto & dnames = getParam<std::vector<std::vector<DistributionName>>>("distributions");
  const auto & nrows = getParam<std::vector<dof_id_type>>("num_rows");
  if (dnames.size() != nrows.size())
    paramError("distributions",
               "There must be a set of distributions for each loop defined by 'num_rows'.");

  // Gather distribution pointers and fill in loop index
  const std::size_t nloop = dnames.size();
  std::vector<std::size_t> loop_index;
  for (const auto & n : make_range(nloop))
    for (const auto & name : dnames[n])
    {
      _distributions.push_back(&getDistributionByName(name));
      loop_index.push_back(n);
    }

  // Compute what row indices need to recompute which columns
  std::vector<dof_id_type> loop_mod(nloop);
  std::partial_sum(nrows.rbegin(), nrows.rend(), loop_mod.rbegin(), std::multiplies<dof_id_type>());
  loop_mod.erase(loop_mod.begin());
  loop_mod.push_back(1);

  // Fill in the mod for each column
  _col_mod.resize(_distributions.size());
  for (const auto j : index_range(_distributions))
    _col_mod[j] = loop_mod[loop_index[j]];

  setNumberOfRows(std::accumulate(nrows.begin(), nrows.end(), 1, std::multiplies<dof_id_type>()));
  setNumberOfCols(_distributions.size());
}

Real
NestedMonteCarloSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  const auto mod = _col_mod[col_index];
  const dof_id_type target_row = std::floor(row_index / mod) * mod;
  const Real rn = getRandStateless(target_row * getNumberOfCols() + col_index);
  return _distributions[col_index]->quantile(rn);
}
