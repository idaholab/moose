//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AIDESGPryLearner.h"
#include <math.h>

registerMooseObject("StochasticToolsApp", AIDESGPryLearner);

InputParameters
AIDESGPryLearner::validParams()
{
  InputParameters params = PMCMCDecision::validParams();
  params.addClassDescription(
      "Perform decision making for Affine Invariant differential MCMC with GPry surrogate.");
  params.addRequiredParam<UserObjectName>("al_gp", "Active learning GP trainer.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluate the trained GP.");
//   params.addParam<ReporterValueName>(
//       "acquisition_function",
//       "acquisition_function",
//       "The values of the acquistion function in the current iteration.");
  params.addParam<ReporterValueName>("convergence_value",
                                     "convergence_value",
                                     "Value to measure convergence of the GPry algorithm.");
  return params;
}

AIDESGPryLearner::AIDESGPryLearner(const InputParameters & parameters)
  : PMCMCDecision(parameters),
    SurrogateModelInterface(this),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcessSurrogate>("gp_evaluator")),
    // _acquisition_function(declareValue<std::vector<Real>>("acquisition_function")),
    _new_samples(_pmcmc->getSamples()),
    _convergence_value(declareValue<Real>("convergence_value"))
{
  _eval_outputs_current.resize(_props);
}

void
AIDESGPryLearner::setupNNGPData(const DenseMatrix<Real> & data_in)
{
  std::vector<Real> tmp;
  if (_var_prior)
    tmp.resize(_priors.size() + 1);
  else
    tmp.resize(_priors.size());
  std::vector<Real> out1(_num_confg_values);
  std::vector<Real> out11(_num_confg_values);
  for (unsigned int i = 0; i < _props; ++i)
  {
    for (unsigned int j = 0; j < _num_confg_values; ++j)
    {
      out1[j] = _outputs_required[j * _props + i];
      out11[j] = _outputs_required1[j * _props + i];
    }
    for (unsigned int j = 0; j < _priors.size(); ++j)
      tmp[j] = data_in(i, j);
    if (_var_prior)
    {
      tmp[_priors.size()] = _new_var_samples[i];
      _noise = std::sqrt(_new_var_samples[i]);
      _gp_inputs.push_back(tmp);
      _gp_outputs.push_back(std::log(_likelihoods[2]->function(out1)) +
                            std::log(_likelihoods[3]->function(out11)));
    }
    else
    {
      _gp_inputs.push_back(tmp);
      _gp_outputs.push_back(std::log(_likelihoods[2]->function(out1)) +
                            std::log(_likelihoods[3]->function(out11)));
    }
  }
}

void
AIDESGPryLearner::computeEvidence(std::vector<Real> & evidence,
                                  const DenseMatrix<Real> & input_matrix)
{
  
  // std::cout << "GP EVIDENCE" << std::endl;
  // std::cout << Moose::stringify(evidence) << std::endl;

  // TRUE COMPUTATIONS
  std::vector<Real> out1(_num_confg_values);
  std::vector<Real> out11(_num_confg_values);
  std::vector<Real> out2(_num_confg_values);
  std::vector<Real> out21(_num_confg_values);
  for (unsigned int i = 0; i < evidence.size(); ++i)
  {
    evidence[i] = 0.0;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      evidence[i] += (std::log(_priors[j]->pdf(input_matrix(i, j))) -
                      std::log(_priors[j]->pdf(_data_prev(i, j))));

    for (unsigned int j = 0; j < _num_confg_values; ++j)
    {
      out1[j] = _outputs_required[j * _props + i];
      out11[j] = _outputs_required1[j * _props + i];
      out2[j] = _outputs_prev[j * _props + i];
      out21[j] = _outputs_prev1[j * _props + i];
    }
    if (_var_prior)
    {
      evidence[i] += (std::log(_var_prior->pdf(_new_var_samples[i])) -
                      std::log(_var_prior->pdf(_var_prev[i])));
      _noise = std::sqrt(_new_var_samples[i]);
      evidence[i] += _likelihoods[0]->function(out1);
      evidence[i] += _likelihoods[1]->function(out11);
      _noise = std::sqrt(_var_prev[i]);
      evidence[i] -= _likelihoods[0]->function(out2);
      evidence[i] -= _likelihoods[1]->function(out21);
    }
    else
      for (unsigned int j = 0; j < _likelihoods.size(); ++j)
        evidence[i] += (_likelihoods[j]->function(out1) - _likelihoods[j]->function(out2));
  }
}

void
AIDESGPryLearner::computeTransitionVector(
    std::vector<Real> & tv, const std::vector<Real> & evidence)
{
  for (unsigned int i = 0; i < tv.size(); ++i)
    tv[i] = std::exp(std::min(evidence[i], 0.0));
}

void
AIDESGPryLearner::nextSamples(std::vector<Real> & req_inputs,
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
  }
}

void
AIDESGPryLearner::computeGPOutput(std::vector<Real> & eval_outputs,
                                  const DenseMatrix<Real> & eval_inputs)
{
  std::vector<Real> tmp;
  if (_var_prior)
    tmp.resize(_priors.size() + 1);
  else
    tmp.resize(_priors.size());
  for (unsigned int i = 0; i < eval_outputs.size(); ++i)
  {
    for (unsigned int j = 0; j < _priors.size(); ++j)
      tmp[j] = eval_inputs(i, j);
    if (_var_prior)
      tmp[_priors.size()] = _new_var_samples[i];
    eval_outputs[i] = _gp_eval.evaluate(tmp);
  }
}

void
AIDESGPryLearner::execute()
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
  _outputs_required1 = _output_value1;
  _local_comm.allgather(_outputs_required);
  _local_comm.allgather(_outputs_required1);

  // Compute the evidence and transition vectors
  std::vector<Real> evidence(_props);
  if (_t_step > _pmcmc->decisionStep())
  {
    computeEvidence(evidence, data_in);
    computeTransitionVector(_tpm, evidence);
    setupNNGPData(data_in);
    if (_t_step > _pmcmc->decisionStep() + 1)
    {
      computeGPOutput(_eval_outputs_current, data_in);
      _convergence_value = 0.0;
      for (unsigned int ii = 0; ii < _props; ++ii)
        _convergence_value += std::abs(_gp_outputs[ii] - _eval_outputs_current[ii]) / (_props);
      std::cout << "_convergence_value " << _convergence_value << std::endl;
    }
    _al_gp.reTrain(_gp_inputs, _gp_outputs);
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
  _outputs_prev1 = _outputs_required1;
  _var_prev = _variance;

  // Track the current step
  _check_step = _t_step;
}
