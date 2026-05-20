//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericActiveLearningSampler.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObject("StochasticToolsApp", GenericActiveLearningSampler);

InputParameters
GenericActiveLearningSampler::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("A generic sampler to support parallel active learning.");
  params.addRequiredParam<unsigned int>(
      "num_parallel_proposals",
      "Number of proposals to make and corresponding subApps executed in "
      "parallel.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>(
      "sorted_indices", "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "num_tries",
      "num_tries>0",
      "Number of samples to propose in each iteration (not all are sent for subApp evals).");
  params.addRequiredParam<std::vector<Real>>("initial_values",
                                             "The starting values of the inputs to be calibrated.");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  return params;
}

GenericActiveLearningSampler::GenericActiveLearningSampler(const InputParameters & parameters)
  : Sampler(parameters),
    TransientInterface(this),
    _num_parallel_proposals(getParam<unsigned int>("num_parallel_proposals")),
    _sorted_indices(getReporterValue<std::vector<unsigned int>>("sorted_indices")),
    _initial_values(getParam<std::vector<Real>>("initial_values")),
    _num_tries(getParam<unsigned int>("num_tries"))
{
  // Filling the `distributions` vector with the user-provided distributions.
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));

  // Setting the number of sampler rows to be equal to the number of parallel proposals
  setNumberOfRows(_num_parallel_proposals);

  // Setting the number of columns in the sampler matrix (equal to the number of distributions)
  setNumberOfCols(_distributions.size());

  // Setting the sizes for the different vectors enabling sampling and selection
  _inputs_all.resize(_num_tries, std::vector<Real>(_distributions.size(), 0.0));
  _new_samples.resize(_num_parallel_proposals);

  setNumberOfRandomSeeds(getParam<unsigned int>("num_random_seeds"));
  setAutoAdvanceGenerators(false);
}

const std::vector<std::vector<Real>> &
GenericActiveLearningSampler::getSampleTries() const
{
  return _inputs_all;
}

void
GenericActiveLearningSampler::executeSetUp()
{
  std::size_t rand_index = 0;
  auto fill_vector = [&](std::vector<Real> & vector)
  {
    vector.resize(getNumberOfCols());
    for (const auto j : make_range(getNumberOfCols()))
      vector[j] = _distributions[j]->quantile(getRand(rand_index++, _t_step + 1));
  };

  /* At the first step, fill_vector is called to advance rand_index so _inputs_all
     gets consistent random values, then _new_samples is overridden with initial values.
     Otherwise, use the samples informed by the GP from the reporter "sorted_indices". */
  for (const auto i : make_range(getNumberOfRows()))
  {
    if (_t_step <= 0)
      fill_vector(_new_samples[i]);
    else
      _new_samples[i] = _inputs_all[_sorted_indices[i]];
  }
  if (_t_step <= 0)
    for (const auto i : make_range(getNumberOfRows()))
      _new_samples[i] = _initial_values;

  /* Finally, generate several new samples randomly for the GP to try and pass it to the
  reporter */
  for (const auto i : make_range(_num_tries))
    fill_vector(_inputs_all[i]);
}

Real
GenericActiveLearningSampler::computeSample(dof_id_type row_index, dof_id_type col_index) const
{
  return _new_samples[row_index][col_index];
}
