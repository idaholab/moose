//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPryTest.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObject("StochasticToolsApp", GPryTest);

InputParameters
GPryTest::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription(
      "Test sampler class for the GPry algorithm.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>("sorted_indices",
                                        "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addRequiredRangeCheckedParam<unsigned int>(
    "num_tries",
    "num_tries>0",
    "Number of samples to propose in each iteration (not all are sent for subApp evals).");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  params.addRequiredParam<unsigned int>(
      "num_parallel_proposals",
      "Number of proposals to make and corresponding subApps executed in "
      "parallel.");
  return params;
}

GPryTest::GPryTest(const InputParameters & parameters)
  : Sampler(parameters),
    TransientInterface(this),
    _sorted_indices(getReporterValue<std::vector<unsigned int>>("sorted_indices")),
    _num_parallel_proposals(getParam<unsigned int>("num_parallel_proposals")),
    _num_tries(getParam<unsigned int>("num_tries"))
{
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));
  setNumberOfRows(_num_parallel_proposals);
  setNumberOfCols(_distributions.size());
  _inputs_all.resize(_num_tries, std::vector<Real>(_distributions.size()));
  _inputs_sto.resize(_num_parallel_proposals, std::vector<Real>(_distributions.size()));
  _sample_vector.resize(_distributions.size());
  setNumberOfRandomSeeds(getParam<unsigned int>("num_random_seeds"));
}

void
GPryTest::fillVector(std::vector<Real> & vector, const unsigned int & seed_value)
{
  for (unsigned int i = 0; i < _distributions.size(); ++i)
    vector[i] = _distributions[i]->quantile(getRand(seed_value));
}

const std::vector<std::vector<Real>> &
GPryTest::getSampleTries() const
{
  return _inputs_all;
}

void
GPryTest::sampleSetUp(const Sampler::SampleMode /*mode*/)
{
  // If we've already done this step, skip
  if (_check_step == _t_step)
    return;

  // If step is 1, randomly generate the samples
  // Else, generate the samples informed by the GP and NN combo from the reporter "sorted_indices"
  for (dof_id_type i = 0; i < _num_parallel_proposals; ++i)
  {
    if (_t_step <= 1)
    {
      fillVector(_sample_vector, _t_step);
      _inputs_sto[i] = _sample_vector;
    }
    else
      _inputs_sto[i] = _inputs_all[_sorted_indices[i]];
  }

  // Finally, generate several new samples randomly for the GP and NN to try and pass it to the
  // reporter
  for (dof_id_type i = 0; i < _num_tries; ++i)
  {
    fillVector(_sample_vector, _t_step);
    _inputs_all[i] = _sample_vector;
  }

  _check_step = _t_step;
}

Real
GPryTest::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  return _inputs_sto[row_index][col_index];
}