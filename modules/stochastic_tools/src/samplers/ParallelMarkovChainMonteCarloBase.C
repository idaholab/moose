//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelMarkovChainMonteCarloBase.h"
#include "AdaptiveMonteCarloUtils.h"
#include "Normal.h"
#include "TruncatedNormal.h"
#include "Uniform.h"
#include "DelimitedFileReader.h"

registerMooseObjectAliased("StochasticToolsApp", ParallelMarkovChainMonteCarloBase, "PMCMCBase");

InputParameters
ParallelMarkovChainMonteCarloBase::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Parallel Markov chain Monte Carlo base.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "prior_distributions",
      "The prior distributions of the parameters to be calibrated.");
  params.addRequiredParam<ReporterName>("seed_inputs",
                                        "Reporter with seed inputs values for the next proposals.");
  params.addRequiredParam<ReporterName>("proposal_std",
                                "Reporter with proposal stds for the next proposals.");
  params.addRequiredParam<unsigned int>("num_parallel_proposals",
                                        "Number of proposals to made and corresponding subApps executed in "
                                        "parallel.");
  // params.addRequiredParam<LikelihoodName>("likelihood", "Name of the likelihood function.");
  params.addRequiredParam<std::vector<Real>>("initial_values", "The starting values of the inputs to be calibrated.");
  params.addRequiredParam<FileName>("file_name", "Name of the CSV file with configuration values.");
  params.addParam<std::string>(
      "file_column_name", "Name of column in CSV file to use, by default first column is used.");
  params.addParam<std::vector<Real>>("std_prop", std::vector<Real>(), "Standard deviations for making the next proposal.");
  params.addParam<std::vector<Real>>("lb", "Lower bounds for making the next proposal.");
  params.addParam<std::vector<Real>>("ub", "Upper bounds for making the next proposal.");
  params.addParam<unsigned int>(
      "num_random_seeds",
      100000,
      "Initialize a certain number of random seeds. Change from the default only if you have to.");
  return params;
}

ParallelMarkovChainMonteCarloBase::ParallelMarkovChainMonteCarloBase(const InputParameters & parameters)
  : Sampler(parameters),
    ReporterInterface(this),
    LikelihoodInterface(this),
    _seed_inputs(getReporterValue<std::vector<Real>>("seed_inputs")),
    _proposal_std(getReporterValue<std::vector<Real>>("proposal_std")),
    _num_parallel_proposals(getParam<unsigned int>("num_parallel_proposals")),
    // _likelihood(getLikelihoodByName(getParam<LikelihoodName>("likelihood"))),
    _initial_values(getParam<std::vector<Real>>("initial_values")),
    _std_prop(getParam<std::vector<Real>>("std_prop")),
    _lb(isParamValid("lb") ? &getParam<std::vector<Real>>("lb") : nullptr),
    _ub(isParamValid("ub") ? &getParam<std::vector<Real>>("ub") : nullptr),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _check_step(0),
    _num_random_seeds(getParam<unsigned int>("num_random_seeds"))
{
  // Filling the `priors` vector with the user-provided distributions.
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("prior_distributions"))
    _priors.push_back(&getDistributionByName(name));

  MooseUtils::DelimitedFileReader reader(getParam<FileName>("file_name"));
  reader.read();
  if (isParamValid("file_column_name"))
    _confg_values = reader.getData(getParam<std::string>("file_column_name"));
  else
    _confg_values = reader.getData(0);

  // Setting the number of sampler rows to be equal to the number of parallel proposals
  setNumberOfRows((_num_parallel_proposals + 1) * _confg_values.size());

  // Setting the number of columns in the sampler matrix (equal to the number of distributions).
  setNumberOfCols(_priors.size()+1);

  // Resizing the new samples vector of vectors
  _new_samples.resize((_num_parallel_proposals + 1) * _confg_values.size());
  for (unsigned int i = 0; i < ((_num_parallel_proposals + 1) * _confg_values.size()); ++i)
    _new_samples[i].resize(_priors.size()+1);
  
  _step_size_sto.resize(_num_parallel_proposals * _confg_values.size());
  
  setNumberOfRandomSeeds(_num_random_seeds);

  _check_step = 0;

  if (_lb && !_ub)
    mooseError("Both lower and upper bounds should be specified.");
  
  if (!_lb && _ub)
    mooseError("Both lower and upper bounds should be specified.");

  if (_std_prop.size() != _priors.size())
    mooseError("The size of the proposal stds should be equal to the number of priors (or tunable params).");

  // _std_prop.resize(_priors.size());
  // _std_prop[0] = 1e-3;
  // _std_prop[1] = 5.0;
  // _std_prop[2] = 0.01;

  // _lb.resize(_priors.size());
  // _lb[0] = 1e-3;
  // _lb[1] = 20.0;
  // _lb[2] = 0.01;

  // _ub.resize(_priors.size());
  // _ub[0] = 9e-2;
  // _ub[1] = 2200.0;
  // _ub[2] = 1.0;
}

dof_id_type
ParallelMarkovChainMonteCarloBase::getNumberOfConfigParams() const
{
  return _confg_values.size();
}

std::vector<Real>
ParallelMarkovChainMonteCarloBase::getAffineStepSize() const
{
  return _step_size_sto;
}

dof_id_type
ParallelMarkovChainMonteCarloBase::getNumParallelProposals() const
{
  return _num_parallel_proposals;
}

void
ParallelMarkovChainMonteCarloBase::sampleSetUp(const SampleMode /*mode*/)
{
  if (_step < 1 || _check_step == _step)
    return;
  _check_step = _step;

  unsigned int seed_value = _step > 0 ? (_step - 1) : 0;
  
  // Filling the new_samples vector of vectors with new proposal samples
  std::vector<Real> tmp(_priors.size() + 1);
  unsigned int count1 = 0;
  if (_step < 3)
  {
    for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
    {
      for (unsigned int i = 0; i < _priors.size(); ++i)
      {
        if (_lb)
          tmp[i] = TruncatedNormal::quantile(getRand(seed_value), _initial_values[i], _std_prop[i], (*_lb)[i], (*_ub)[i]);
        else
          tmp[i] = Normal::quantile(getRand(seed_value), _initial_values[i], _std_prop[i]);
      }
      for (unsigned int i = 0; i < _confg_values.size(); ++i)
      {
        tmp[_priors.size()] = _confg_values[i];
        _new_samples[count1] = tmp;
        count1 += 1;
      }
    }
    for (unsigned int i = 0; i < _priors.size(); ++i)
      tmp[i] = _initial_values[i];
    for (unsigned int i = 0; i < _confg_values.size(); ++i)
    {
      tmp[_priors.size()] = _confg_values[i];
      _new_samples[_num_parallel_proposals * _confg_values.size() + i] = tmp;
    }
  }
  else
  {
    // Real std_tmp;
    std::cout << "_seed_inputs " << Moose::stringify(_seed_inputs) << std::endl;
    for (unsigned int j = 0; j < _num_parallel_proposals; ++j)
    {
      for (unsigned int i = 0; i < _priors.size(); ++i)
      {
        // if (_step > 10000)
        //   std_tmp = _proposal_std[i];
        // else
        //   std_tmp = 0.15; // 1e-5; // 5e-5
        if (_lb)
          tmp[i] = TruncatedNormal::quantile(getRand(seed_value), _seed_inputs[i], _std_prop[i], (*_lb)[i], (*_ub)[i]);
        else
          tmp[i] = Normal::quantile(getRand(seed_value), _seed_inputs[i], _std_prop[i]);
      }
      for (unsigned int i = 0; i < _confg_values.size(); ++i)
      {
        tmp[_priors.size()] = _confg_values[i];
        _new_samples[count1] = tmp;
        count1 += 1;
      }
    }
    for (unsigned int i = 0; i < _priors.size(); ++i)
      tmp[i] = _seed_inputs[i];
    for (unsigned int i = 0; i < _confg_values.size(); ++i)
    {
      tmp[_priors.size()] = _confg_values[i];
      _new_samples[_num_parallel_proposals * _confg_values.size() + i] = tmp;
    }
  }
  for (unsigned int i = 0; i < ((_num_parallel_proposals + 1) * _confg_values.size()); ++i)
    std::cout << Moose::stringify(_new_samples[i]) << std::endl;
}

Real
ParallelMarkovChainMonteCarloBase::computeSample(dof_id_type row_index, dof_id_type col_index)
{

  std::vector<Real> init;
  // init = {1e-9, 1e4, 1e-9, 1e4, 1e-9, 1e4, 643};
  // init = {-20.723, 11.0, -20.723, 11.0, -20.723, 11.0, 643};
  init = {0.05, 0.05, 8};

  // std::cout << Moose::stringify(_new_samples[row_index]) << std::endl;  
// std::cout << "Here *****" << std::endl;

  if (_step == 0)
  {
    if (col_index < 6)
      return std::exp(init[col_index]);
    else
      return (init[col_index]);
  }
  else
  {
    if (col_index < 6)
      return std::exp(_new_samples[row_index][col_index]);
    else
      return (_new_samples[row_index][col_index]);
  }
}
