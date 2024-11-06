//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPAffineInvariantDifferentialDecision.h"

registerMooseObject("StochasticToolsApp", GPAffineInvariantDifferentialDecision);

InputParameters
GPAffineInvariantDifferentialDecision::validParams()
{
  InputParameters params = PMCMCDecision::validParams();
  params.addClassDescription("Perform decision making for Affine Invariant differential MCMC using a GP.");
  params.addParam<bool>(
      "correct_GP_output",
      false,
      "Bool to correct the GP predicted output to adjust for the right variance.");
  params.addParam<Real>(
      "incorrect_variance",
      1.0,
      "The incorrect variance value used during active learning.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluate the trained GP.");
  params.addParam<ReporterValueName>(
      "estimated_loglikelihood", "estimated_loglikelihood", "The GP estimated log-likelihood.");
  return params;
}

GPAffineInvariantDifferentialDecision::GPAffineInvariantDifferentialDecision(
    const InputParameters & parameters)
  : PMCMCDecision(parameters),
    SurrogateModelInterface(this),
    _correct_GP_output(getParam<bool>("correct_GP_output")),
    _incorrect_variance(getParam<Real>("incorrect_variance")),
    _aides(dynamic_cast<const AffineInvariantDES *>(&_sampler)),
    _gp_eval(getSurrogateModel<GaussianProcessSurrogate>("gp_evaluator")),
    _estimated_loglikelihood(declareValue<std::vector<Real>>("estimated_loglikelihood"))
{
  // Check whether the selected sampler is a differential evolution sampler or not
  if (!_aides)
    paramError("sampler", "The selected sampler is not of type AffineInvariantDES.");

  _estimated_loglikelihood.resize(_props);
}

Real
GPAffineInvariantDifferentialDecision::correctGP(const Real & GPoutput, const Real & trueVariance)
{
  Real correctGP = GPoutput;
  correctGP -= _num_confg_values * std::log(1.0 / (std::sqrt(2.0 * _incorrect_variance * M_PI)));
  correctGP = correctGP * _incorrect_variance / trueVariance;
  correctGP += _num_confg_values * std::log(1.0 / (std::sqrt(2.0 * trueVariance * M_PI)));
  return correctGP;
}

void
GPAffineInvariantDifferentialDecision::computeEvidence(std::vector<Real> & evidence,
                                                       const DenseMatrix<Real> & input_matrix)
{
  std::vector<Real> tmp;
  tmp.resize(_priors.size());
  Real estimated_evidence;
  Real GP_pred;
  for (unsigned int i = 0; i < evidence.size(); ++i)
  {
    estimated_evidence = 0.0;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      estimated_evidence += (std::log(_priors[j]->pdf(input_matrix(i, j))) -
                             std::log(_priors[j]->pdf(_data_prev(i, j))));

    for (unsigned int j = 0; j < _priors.size(); ++j)
      tmp[j] = input_matrix(i, j);
    if (_var_prior)
      estimated_evidence += (std::log(_var_prior->pdf(_new_var_samples[i])) -
                             std::log(_var_prior->pdf(_var_prev[i])));
    _estimated_loglikelihood[i] = _gp_eval.evaluate(tmp);
    GP_pred = _estimated_loglikelihood[i];
    estimated_evidence +=
        (_var_prior && _correct_GP_output) ? correctGP(GP_pred, _new_var_samples[i]) : GP_pred;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      tmp[j] = _data_prev(i, j);
    GP_pred = _gp_eval.evaluate(tmp);
    estimated_evidence -=
        (_var_prior && _correct_GP_output) ? correctGP(GP_pred, _var_prev[i]) : GP_pred;
    evidence[i] = estimated_evidence;
  }
}

void
GPAffineInvariantDifferentialDecision::computeTransitionVector(std::vector<Real> & tv,
                                                               const std::vector<Real> & evidence)
{
  for (unsigned int i = 0; i < tv.size(); ++i)
    tv[i] = std::exp(std::min(evidence[i], 0.0));
}

void
GPAffineInvariantDifferentialDecision::nextSamples(std::vector<Real> & req_inputs,
                                                   DenseMatrix<Real> & input_matrix,
                                                   const std::vector<Real> & tv,
                                                   const unsigned int & parallel_index)
{
  if (tv[parallel_index] >= _rnd_vec[parallel_index])
  {
    for (unsigned int k = 0; k < _sampler.getNumberOfCols(); ++k)
      req_inputs[k] = input_matrix(parallel_index, k);
    _variance[parallel_index] = _new_var_samples[parallel_index];
  }
  else
  {
    for (unsigned int k = 0; k < _sampler.getNumberOfCols(); ++k)
    {
      req_inputs[k] = _data_prev(parallel_index, k);
      input_matrix(parallel_index, k) = _data_prev(parallel_index, k);
    }
    if (_var_prior)
      _variance[parallel_index] = _var_prev[parallel_index];
  }
}

void
GPAffineInvariantDifferentialDecision::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _t_step)
  {
    _check_step = _t_step;
    return;
  }

  // Gather inputs from the sampler
  DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      data_in(ss, j) = data[j];
  }
  _local_comm.sum(data_in.get_values());

  // Compute the evidence and transition vectors
  std::vector<Real> evidence(_props);
  if (_t_step > _pmcmc->decisionStep())
  {
    computeEvidence(evidence, data_in);
    computeTransitionVector(_tpm, evidence);
  }
  else
    _tpm.assign(_props, 1.0);

  // Accept/reject the proposed samples
  std::vector<Real> req_inputs(_sampler.getNumberOfCols());
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
