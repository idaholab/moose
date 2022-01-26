//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
  for (const auto & n : make_range(nloop))
    for (const auto & name : dnames[n])
    {
      _distributions.push_back(&getDistributionByName(name));
      _loop_index.push_back(n);
    }

  // Compute what row indices need to recompute which columns
  _loop_mod.resize(nloop);
  std::partial_sum(
      nrows.rbegin(), nrows.rend(), _loop_mod.rbegin(), std::multiplies<dof_id_type>());
  _loop_mod.erase(_loop_mod.begin());
  _loop_mod.push_back(1);

  // Allocate row storage
  _row_data.resize(_distributions.size());

  setNumberOfRows(std::accumulate(nrows.begin(), nrows.end(), 1, std::multiplies<dof_id_type>()));
  setNumberOfCols(_distributions.size());
}

void
NestedMonteCarloSampler::sampleSetUp(const SampleMode mode)
{
  if (mode == Sampler::SampleMode::GLOBAL || getNumberOfRows() == 0)
    return;

  dof_id_type curr_row = 0;
  for (const auto & mod : _loop_mod)
  {
    if (getLocalRowBegin() % mod == 0)
      break;

    const dof_id_type target_row = std::floor(getLocalRowBegin() / mod) * mod;
    advanceGenerators((target_row - curr_row) * getNumberOfCols());
    for (const auto & j : make_range(getNumberOfCols()))
      computeSample(target_row, j);
    curr_row = target_row + 1;
  }
  restoreGeneratorState();
}

Real
NestedMonteCarloSampler::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  const Real rn = getRand();
  const auto & loop = _loop_index[col_index];
  if (row_index % _loop_mod[loop] == 0)
    _row_data[col_index] = _distributions[col_index]->quantile(rn);
  return _row_data[col_index];
}
