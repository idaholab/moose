//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "AffineInvariantDifferentialDecisionwithGPry.h"

registerMooseObject("StochasticToolsApp", AffineInvariantDifferentialDecisionwithGPry);

InputParameters
AffineInvariantDifferentialDecisionwithGPry::validParams()
{
  InputParameters params = PMCMCDecision::validParams();
  params.addClassDescription("Perform decision making for Affine Invariant differential MCMC with GPry surrogate.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluate the trained GP.");
  params.addRequiredParam<UserObjectName>("nn_evaluator", "Evaluate the trained NN.");
  //   params.suppressParameter<ReporterName>("output_value");
  return params;
}

AffineInvariantDifferentialDecisionwithGPry::AffineInvariantDifferentialDecisionwithGPry(
    const InputParameters & parameters)
  : PMCMCDecision(parameters),
    SurrogateModelInterface(this),
    _gp_eval(getSurrogateModel<GaussianProcess>("gp_evaluator")),
    _nn_eval(getSurrogateModel<LibtorchANNSurrogate>("nn_evaluator")),
    _new_samples(_pmcmc->getSamples())
{
}

void
AffineInvariantDifferentialDecisionwithGPry::computeEvidence(std::vector<Real> & evidence,
                                                             const DenseMatrix<Real> & input_matrix)
{
  std::vector<Real> tmp;
  if (_var_prior)
    tmp.resize(_priors.size() + 1);
  else
    tmp.resize(_priors.size());

  Real estimated_evidence;
//   Real estimated_class;
  for (unsigned int i = 0; i < evidence.size(); ++i)
  {
    estimated_evidence = 0.0;

    for (unsigned int j = 0; j < _priors.size(); ++j)
      tmp[j] = input_matrix(i, j);
    if (_var_prior)
      tmp[_priors.size()] = _new_var_samples[i];
    // estimated_class = _nn_eval.evaluate(tmp);
    estimated_evidence +=
        _gp_eval.evaluate(tmp); // (estimated_class > 0.5) ? _gp_eval.evaluate(tmp) : -10000.0;

    for (unsigned int j = 0; j < _priors.size(); ++j)
      tmp[j] = _data_prev(i, j);
    if (_var_prior)
      tmp[_priors.size()] = _var_prev[i];
    // estimated_class = _nn_eval.evaluate(tmp);
    estimated_evidence -=
        _gp_eval.evaluate(tmp); // (estimated_class > 0.5) ? _gp_eval.evaluate(tmp) : -10000.0;

    // for (unsigned int j = 0; j < _priors.size(); ++j)
    //   estimated_evidence += (std::log(_priors[j]->pdf(input_matrix(i, j))) -
    //                          std::log(_priors[j]->pdf(_data_prev(i, j))));

    evidence[i] = estimated_evidence;
  }
}

void
AffineInvariantDifferentialDecisionwithGPry::computeTransitionVector(
    std::vector<Real> & tv, const std::vector<Real> & evidence)
{
  for (unsigned int i = 0; i < tv.size(); ++i)
    tv[i] = std::exp(std::min(evidence[i], 0.0));
}

void
AffineInvariantDifferentialDecisionwithGPry::nextSamples(std::vector<Real> & req_inputs,
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
AffineInvariantDifferentialDecisionwithGPry::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _t_step)
  {
    _check_step = _t_step;
    return;
  }
  
  // Gather inputs and outputs from the sampler and subApps
//   DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
//   for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
//   {
//     const auto data = _sampler.getNextLocalRow();
//     for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
//       data_in(ss, j) = data[j];
//   }
//   _local_comm.sum(data_in.get_values());
  DenseMatrix<Real> data_in(_props, _priors.size());
  for (dof_id_type ss = 0; ss < _props; ++ss)
  {
    for (unsigned int j = 0; j < _priors.size(); ++j)
      data_in(ss, j) = _new_samples[ss][j];
  }

  // Compute the evidence and transition vectors
  std::vector<Real> evidence(_props);
  if (_t_step > _pmcmc->decisionStep())
  {
    // std::cout << Moose::stringify(_new_samples) << std::endl;
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
  _var_prev = _variance;

  // Track the current step
  _check_step = _t_step;
}

#endif
