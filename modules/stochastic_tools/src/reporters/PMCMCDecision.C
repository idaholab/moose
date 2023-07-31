//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PMCMCDecision.h"
#include "Sampler.h"
#include "DenseMatrix.h"

registerMooseObject("StochasticToolsApp", PMCMCDecision);

InputParameters
PMCMCDecision::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += LikelihoodInterface::validParams();
  params.addClassDescription("Generic reporter which decides whether or not to accept a proposed "
                             "sample in parallel Markov chain Monte Carlo type of algorithms.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>(
      "outputs_required",
      "outputs_required",
      "Modified value of the model output from this reporter class.");
  params.addParam<ReporterValueName>("inputs", "inputs", "Uncertain inputs to the model.");
  params.addParam<ReporterValueName>("tpm", "tpm", "The transition probability matrix.");
  params.addParam<ReporterValueName>("variance", "variance", "Model variance term.");
  params.addParam<ReporterValueName>(
      "noise", "noise", "Model noise term to pass to Likelihoods object.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<std::vector<UserObjectName>>("likelihoods", "Names of likelihoods.");
  return params;
}

PMCMCDecision::PMCMCDecision(const InputParameters & parameters)
  : GeneralReporter(parameters),
    LikelihoodInterface(parameters),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _outputs_required(declareValue<std::vector<Real>>("outputs_required")),
    _inputs(declareValue<std::vector<std::vector<Real>>>("inputs")),
    _tpm(declareValue<std::vector<Real>>("tpm")),
    _variance(declareValue<std::vector<Real>>("variance")),
    _noise(declareValue<Real>("noise")),
    _sampler(getSampler("sampler")),
    _pmcmc(dynamic_cast<const PMCMCBase *>(&_sampler)),
    _rnd_vec(_pmcmc->getRandomNumbers()),
    _new_var_samples(_pmcmc->getVarSamples()),
    _priors(_pmcmc->getPriors()),
    _var_prior(_pmcmc->getVarPrior()),
    _local_comm(_sampler.getLocalComm()),
    _check_step(std::numeric_limits<int>::max())
{
  // Filling the `likelihoods` vector with the user-provided distributions.
  for (const UserObjectName & name : getParam<std::vector<UserObjectName>>("likelihoods"))
    _likelihoods.push_back(getLikelihoodFunctionByName(name));

  // Check whether the selected sampler is an MCMC sampler or not
  if (!_pmcmc)
    paramError("sampler", "The selected sampler is not of type MCMC.");

  // Fetching the sampler characteristics
  _props = _pmcmc->getNumParallelProposals();
  _num_confg_values = _pmcmc->getNumberOfConfigValues();
  _num_confg_params = _pmcmc->getNumberOfConfigParams();

  // Resizing the data arrays to transmit to the output file
  _inputs.resize(_props);
  for (unsigned int i = 0; i < _props; ++i)
    _inputs[i].resize(_sampler.getNumberOfCols() - _num_confg_params);
  _outputs_required.resize(_sampler.getNumberOfRows());
  _tpm.resize(_props);
  _variance.resize(_props);
}

void
PMCMCDecision::computeEvidence(std::vector<Real> & evidence, const DenseMatrix<Real> & input_matrix)
{
  std::vector<Real> out1(_num_confg_values);
  std::vector<Real> out2(_num_confg_values);
  for (unsigned int i = 0; i < evidence.size(); ++i)
  {
    evidence[i] = 0.0;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      evidence[i] += (std::log(_priors[j]->pdf(input_matrix(i, j))) -
                      std::log(_priors[j]->pdf(_data_prev(i, j))));
    for (unsigned int j = 0; j < _num_confg_values; ++j)
    {
      out1[j] = _outputs_required[j * _props + i];
      out2[j] = _outputs_prev[j * _props + i];
    }
    if (_var_prior)
    {
      evidence[i] += (std::log(_var_prior->pdf(_new_var_samples[i])) -
                      std::log(_var_prior->pdf(_var_prev[i])));
      _noise = std::sqrt(_new_var_samples[i]);
      for (unsigned int j = 0; j < _likelihoods.size(); ++j)
        evidence[i] += _likelihoods[j]->function(out1);
      _noise = std::sqrt(_var_prev[i]);
      for (unsigned int j = 0; j < _likelihoods.size(); ++j)
        evidence[i] -= _likelihoods[j]->function(out2);
    }
    else
      for (unsigned int j = 0; j < _likelihoods.size(); ++j)
        evidence[i] += (_likelihoods[j]->function(out1) - _likelihoods[j]->function(out2));
  }
}

void
PMCMCDecision::computeTransitionVector(std::vector<Real> & tv,
                                       const std::vector<Real> & /*evidence*/)
{
  tv.assign(_props, 1.0);
}

void
PMCMCDecision::nextSamples(std::vector<Real> & req_inputs,
                           DenseMatrix<Real> & input_matrix,
                           const std::vector<Real> & tv,
                           const unsigned int & parallel_index)
{
  if (tv[parallel_index] >= _rnd_vec[parallel_index])
  {
    for (unsigned int k = 0; k < _sampler.getNumberOfCols() - _num_confg_params; ++k)
      req_inputs[k] = input_matrix(parallel_index, k);
    _variance[parallel_index] = _new_var_samples[parallel_index];
  }
  else
  {
    for (unsigned int k = 0; k < _sampler.getNumberOfCols() - _num_confg_params; ++k)
    {
      req_inputs[k] = _data_prev(parallel_index, k);
      input_matrix(parallel_index, k) = _data_prev(parallel_index, k);
    }
    if (_var_prior)
      _variance[parallel_index] = _var_prev[parallel_index];
    for (unsigned int k = 0; k < _num_confg_values; ++k)
      _outputs_required[k * _props + parallel_index] = _outputs_prev[k * _props + parallel_index];
  }
}

void
PMCMCDecision::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _t_step)
  {
    _check_step = _t_step;
    return;
  }

  // Gather inputs and outputs from the sampler and subApps
  DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      data_in(ss, j) = data[j];
  }
  _local_comm.sum(data_in.get_values());
  _outputs_required = _output_value;
  _local_comm.allgather(_outputs_required);

  // Compute the evidence and transition vectors
  std::vector<Real> evidence(_props);
  if (_t_step > _pmcmc->decisionStep())
  {
    computeEvidence(evidence, data_in);
    computeTransitionVector(_tpm, evidence);
  }
  else
    _tpm.assign(_props, 1.0);

  // Accept/reject the proposed samples and assign the correct outputs
  std::vector<Real> req_inputs(_sampler.getNumberOfCols() - _num_confg_params);
  for (unsigned int i = 0; i < _props; ++i)
  {
    nextSamples(req_inputs, data_in, _tpm, i);
    _inputs[i] = req_inputs;
  }

  // Compute the next seeds to facilitate proposals (not always required)
  nextSeeds();

  // Store data from previous step
  _data_prev = data_in;
  _outputs_prev = _outputs_required;
  _var_prev = _variance;

  // Track the current step
  _check_step = _t_step;
}
