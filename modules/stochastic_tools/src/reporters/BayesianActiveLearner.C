//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BayesianActiveLearner.h"

registerMooseObject("StochasticToolsApp", BayesianActiveLearner);

InputParameters
BayesianActiveLearner::validParams()
{
  InputParameters params = GenericActiveLearner::validParams();
  params += LikelihoodInterface::validParams();
  params.addClassDescription(
      "A reporter to support parallel active learning for Bayesian UQ tasks.");
  params.addRequiredParam<std::vector<UserObjectName>>("likelihoods", "Names of likelihoods.");
  params.addParam<ReporterValueName>(
      "noise", "noise", "Model noise term to pass to Likelihoods object.");
  return params;
}

BayesianActiveLearner::BayesianActiveLearner(const InputParameters & parameters)
  : GenericActiveLearner(parameters),
    LikelihoodInterface(parameters),
    _bayes_al_sampler(dynamic_cast<const BayesianActiveLearningSampler *>(&_sampler)),
    _new_var_samples(_bayes_al_sampler->getVarSamples()),
    _inputs_test(_bayes_al_sampler->getSampleTries()),
    _var_prior(_bayes_al_sampler->getVarPrior()),
    _var_test(_bayes_al_sampler->getVarSampleTries()),
    _noise(declareValue<Real>("noise"))
{
  // Check whether the selected sampler is BayesianActiveLearningSampler or not
  if (!_bayes_al_sampler)
    paramError("sampler", "The selected sampler is not of type BayesianActiveLearningSampler.");

  // Filling the `likelihoods` vector with the user-provided distributions.
  for (const UserObjectName & name : getParam<std::vector<UserObjectName>>("likelihoods"))
    _likelihoods.push_back(getLikelihoodFunctionByName(name));

  _num_confg_values = _bayes_al_sampler->getNumberOfConfigValues();
  _num_confg_params = _bayes_al_sampler->getNumberOfConfigParams();

  // Resize the length scales depending upon variance is included
  if (_var_prior)
    _length_scales.resize(_n_dim + 1);
  else
    _length_scales.resize(_n_dim);

  // Resize the log-likelihood vector to the number of parallel proposals
  _log_likelihood.resize(_props);

  // Fetching the sampler characteristics
  _n_dim = _sampler.getNumberOfCols();
  _props = _bayes_al_sampler->getNumParallelProposals();

  // Setting up the variable sizes to facilitate active learning
  _gp_outputs_test.resize(_inputs_test.size());
  _gp_std_test.resize(_inputs_test.size());
  _acquisition_value.resize(_props);
  _length_scales.resize(_n_dim);
  _eval_outputs_current.resize(_props);
  _generic.resize(1);
}

void
BayesianActiveLearner::setupGPData(const std::vector<Real> & data_out,
                                   const DenseMatrix<Real> & data_in)
{
  std::vector<Real> tmp;
  computeLogLikelihood(data_out);
  if (_var_prior)
    tmp.resize(_n_dim + 1);
  else
    tmp.resize(_n_dim);
  for (unsigned int i = 0; i < _props; ++i)
  {
    for (unsigned int j = 0; j < _n_dim; ++j)
      tmp[j] = data_in(i, j);
    if (_var_prior)
      tmp[_n_dim] = _new_var_samples[i];
    if (_log_likelihood[i] == _log_likelihood[i])
    {
      _gp_inputs.push_back(tmp);
      _gp_outputs.push_back(_log_likelihood[i]);
    }
  }
}

void
BayesianActiveLearner::computeLogLikelihood(const std::vector<Real> & data_out)
{
  std::vector<Real> out1(_num_confg_values);
  for (unsigned int i = 0; i < _props; ++i)
  {
    _log_likelihood[i] = 0.0;
    for (unsigned int j = 0; j < _num_confg_values; ++j)
    {
      out1[j] = data_out[j * _props + i];
    }
    if (_var_prior)
    {
      _noise = std::sqrt(_new_var_samples[i]);
      _log_likelihood[i] +=
          _likelihoods[0]->function(out1);
    }
    else
      _log_likelihood[i] +=
          _likelihoods[0]->function(out1);
  }
}

void
BayesianActiveLearner::computeConvergenceValue()
{
  unsigned int num_valid = 0;
  for (unsigned int ii = 0; ii < _props; ++ii)
  {
    if (_log_likelihood[ii] == _log_likelihood[ii])
    {
      _convergence_value += Utility::pow<2>(_log_likelihood[ii] - _eval_outputs_current[ii]);
      ++num_valid;
    }
  }
  _convergence_value = std::sqrt(_convergence_value) / num_valid;
}

void
BayesianActiveLearner::evaluateGPTest()
{
  std::vector<Real> tmp;
  if (_var_prior)
    tmp.resize(_n_dim + 1);
  else
    tmp.resize(_n_dim);
  for (unsigned int i = 0; i < _gp_outputs_test.size(); ++i)
  {
    for (unsigned int j = 0; j < _n_dim; ++j)
      tmp[j] = _inputs_test[i][j];
    if (_var_prior)
      tmp[_n_dim] = _var_test[i];
    _gp_outputs_test[i] = _gp_eval.evaluate(tmp, _gp_std_test[i]);
  }
}

void
BayesianActiveLearner::includeAdditionalInputs()
{
  _inputs_test_modified.resize(_inputs_test.size());
  std::vector<Real> tmp;
  if (_var_prior)
    tmp.resize(_n_dim + 1);
  else
    tmp.resize(_n_dim);
  for (unsigned int i = 0; i < _inputs_test.size(); ++i)
  {
    for (unsigned int j = 0; j < _n_dim; ++j)
      tmp[j] = _inputs_test[i][j];
    if (_var_prior)
      tmp[_n_dim] = _var_test[i];
    _inputs_test_modified[i] = tmp;
  }
}

