//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MonteCarloSampler.h"
#include "Distribution.h"

registerMooseObjectAliased("StochasticToolsApp", MonteCarloSampler, "MonteCarlo");
registerMooseObjectReplaced("StochasticToolsApp",
                            MonteCarloSampler,
                            "07/01/2020 00:00",
                            MonteCarlo);

InputParameters
MonteCarloSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Monte Carlo Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The number of rows per matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  return params;
}

MonteCarloSampler::MonteCarloSampler(const InputParameters & parameters)
  : Sampler(parameters),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions"))
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
}

Real
MonteCarloSampler::computeSample(dof_id_type /*row_index*/, dof_id_type col_index)
{
  return _distributions[col_index]->quantile(getRand());
}
