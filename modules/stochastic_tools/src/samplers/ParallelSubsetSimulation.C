//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelSubsetSimulation.h"
#include "AdaptiveMonteCarloUtils.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObject("StochasticToolsApp", ParallelSubsetSimulation);

InputParameters
ParallelSubsetSimulation::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Parallel Subset Simulation sampler.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>("output_reporter",
                                        "Reporter with results of samples created by the SubApp.");
  params.addRequiredParam<ReporterName>("inputs_reporter", "Reporter with input parameters.");
  params.addRangeCheckedParam<Real>("subset_probability",
                                    0.1,
                                    "subset_probability>0 & subset_probability<=1",
                                    "Conditional probability of each subset");
  params.addRequiredParam<unsigned int>("num_samplessub", "Number of samples per subset");
  params.addRequiredParam<unsigned int>("num_subsets", "Number of desired subsets");
  params.addParam<unsigned int>("num_parallel_chains",
                                "Number of Markov chains to run in parallel, default is based on "
                                "the number of processors used.");
  params.addParam<bool>("use_absolute_value", false, "Use absolute value of the sub app output");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  return params;
}

ParallelSubsetSimulation::ParallelSubsetSimulation(const InputParameters & parameters)
  : Sampler(parameters),
    ReporterInterface(this),
    _num_samplessub(getParam<unsigned int>("num_samplessub")),
    _num_subsets(getParam<unsigned int>("num_subsets")),
    _use_absolute_value(getParam<bool>("use_absolute_value")),
    _subset_probability(getParam<Real>("subset_probability")),
    _num_random_seeds(getParam<unsigned int>("num_random_seeds")),
    _outputs(getReporterValue<std::vector<Real>>("output_reporter")),
    _inputs(getReporterValue<std::vector<std::vector<Real>>>("inputs_reporter")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _count_max(std::floor(1 / _subset_probability)),
    _check_step(0),
    _subset(0),
    _is_sampling_completed(false)
{
  // Fixing the number of rows to the number of processors
  const dof_id_type nchains = isParamValid("num_parallel_chains")
                                  ? getParam<unsigned int>("num_parallel_chains")
                                  : n_processors() / _min_procs_per_row;
  setNumberOfRows(nchains);
  if ((_num_samplessub / nchains) % _count_max > 0)
    mooseError("Number of model evaluations per chain per subset (",
               _num_samplessub / nchains,
               ") should be a multiple of requested chain length (",
               _count_max,
               ").");

  // Filling the `distributions` vector with the user-provided distributions.
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _distributions.push_back(&getDistributionByName(name));

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_distributions.size());

  /* `inputs_sto` is a member variable that aids in deciding the next set of samples
  in the Subset Simulation algorithm by storing the input parameter values*/
  _inputs_sto.resize(_distributions.size(), std::vector<Real>(_num_samplessub, 0.0));
  _outputs_sto.resize(_num_samplessub, 0.0);

  /* `inputs_sorted` is a member variable which also aids in deciding the next set of samples
  in the Subset Simulation algorithm by storing the sorted input parameter values
  by their corresponding output values*/
  _inputs_sorted.resize(_distributions.size());

  /* `markov_seed` is a member variable to store the seed input values for proposing
  the next set of Markov chain samples.*/
  _markov_seed.resize(_distributions.size());

  setNumberOfRandomSeeds(_num_random_seeds);
}

const unsigned int &
ParallelSubsetSimulation::getNumSamplesSub() const
{
  return _num_samplessub;
}

const bool &
ParallelSubsetSimulation::getUseAbsoluteValue() const
{
  return _use_absolute_value;
}

const Real &
ParallelSubsetSimulation::getSubsetProbability() const
{
  return _subset_probability;
}

void
ParallelSubsetSimulation::sampleSetUp(const SampleMode mode)
{
  if (_step <= 1 || _check_step == _step)
    return;
  _check_step = _step;

  if (_is_sampling_completed)
    mooseError("Internal bug: the adaptive sampling is supposed to be completed but another sample "
               "has been requested.");

  _subset = ((_step - 1) * getNumberOfRows()) / _num_samplessub;
  const unsigned int sub_ind = (_step - 1) - (_num_samplessub / getNumberOfRows()) * _subset;
  const unsigned int offset = sub_ind * getNumberOfRows();

  // Get and store the accepted samples input across all the procs from the previous step
  for (dof_id_type j = 0; j < _distributions.size(); ++j)
    for (dof_id_type ss = 0; ss < getNumberOfRows(); ++ss)
      _inputs_sto[j][ss + offset] = Normal::quantile(_distributions[j]->cdf(_inputs[j][ss]), 0, 1);

  // Get the accepted sample outputs across all the procs from the previous step
  std::vector<Real> tmp =
      _use_absolute_value ? AdaptiveMonteCarloUtils::computeVectorABS(_outputs) : _outputs;
  if (mode == Sampler::SampleMode::GLOBAL)
    _communicator.allgather(tmp);
  else
    _local_comm.allgather(tmp);
  for (dof_id_type ss = 0; ss < getNumberOfRows(); ++ss)
    _outputs_sto[ss + offset] = tmp[ss];

  // These are the subsequent subsets which use Markov Chain Monte Carlo sampling scheme
  if (_subset > 0)
  {
    // Check whether the subset index has changed
    if (sub_ind == 0)
    {
      // _inputs_sorted contains the input values corresponding to the largest po percentile
      // output values
      _inputs_sorted = AdaptiveMonteCarloUtils::sortInput(
          _inputs_sto, _outputs_sto, _num_samplessub, _subset_probability);
    }

    // Reinitialize the starting inputs values for the next set of Markov chains
    if (sub_ind % _count_max == 0)
    {
      const unsigned int soffset = (sub_ind / _count_max) * getNumberOfRows();
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _markov_seed[j].assign(_inputs_sorted[j].begin() + soffset + getLocalRowBegin(),
                               _inputs_sorted[j].begin() + soffset + getLocalRowEnd());
    }
    // Otherwise, use the previously accepted input values to propose the next set of input
    // values
    else
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _markov_seed[j].assign(_inputs_sto[j].begin() + offset + getLocalRowBegin(),
                               _inputs_sto[j].begin() + offset + getLocalRowEnd());
    }
  }

  // check if we have completed the last sample (sub_ind == _num_samplessub /getNumberOfRows() - 1)
  // of the last subset (_subset == _num_subsets - 1)
  if (_subset == _num_subsets - 1 && sub_ind == _num_samplessub / getNumberOfRows() - 1)
    _is_sampling_completed = true;
}

Real
ParallelSubsetSimulation::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  unsigned int seed_value = _step > 0 ? (_step - 1) * 2 : 0;
  Real val;

  if (_subset == 0)
    val = getRand(seed_value);
  else
  {
    const dof_id_type loc_ind = row_index - getLocalRowBegin();
    const Real rv = Normal::quantile(getRand(seed_value), _markov_seed[col_index][loc_ind], 1.0);
    const Real acceptance_ratio = std::log(Normal::pdf(rv, 0, 1)) -
                                  std::log(Normal::pdf(_markov_seed[col_index][loc_ind], 0, 1));
    const Real new_sample = acceptance_ratio > std::log(getRand(seed_value + 1))
                                ? rv
                                : _markov_seed[col_index][loc_ind];
    val = Normal::cdf(new_sample, 0, 1);
  }

  return _distributions[col_index]->quantile(val);
}
