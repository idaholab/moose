//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef LIBTORCH_ENABLED

#include "BayesianGPryLearner.h"
#include "Sampler.h"
#include "DenseMatrix.h"
#include "AdaptiveMonteCarloUtils.h"
#include "StochasticToolsUtils.h"

registerMooseObject("StochasticToolsApp", BayesianGPryLearner);

InputParameters
BayesianGPryLearner::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += LikelihoodInterface::validParams();
  params.addClassDescription("Fast Bayesian inference with the GPry algorithm by El Gammal et al. "
                             "2023: NN and GP training step.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>(
      "output_comm", "output_comm", "Modified value of the model output from this reporter class.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<UserObjectName>("al_nn", "Active learning NN trainer.");
  params.addRequiredParam<UserObjectName>("nn_evaluator", "Evaluate the trained NN.");
  params.addRequiredParam<UserObjectName>("al_gp", "Active learning GP trainer.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluate the trained GP.");
  params.addParam<ReporterValueName>(
      "sorted_indices",
      "sorted_indices",
      "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addRequiredParam<std::vector<UserObjectName>>("likelihoods", "Names of likelihoods.");
  return params;
}

BayesianGPryLearner::BayesianGPryLearner(const InputParameters & parameters)
  : GeneralReporter(parameters),
    LikelihoodInterface(parameters),
    SurrogateModelInterface(this),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _output_comm(declareValue<std::vector<Real>>("output_comm")),
    _sampler(getSampler("sampler")),
    _gpry_sampler(dynamic_cast<const BayesianGPrySampler *>(&_sampler)),
    _sorted_indices(declareValue<std::vector<unsigned int>>(
        "sorted_indices", std::vector<unsigned int>(_sampler.getNumberOfRows(), 0))),
    _inputs_all(_gpry_sampler->getSampleTries()),
    _var_all(_gpry_sampler->getVarSampleTries()),
    _al_nn(getUserObject<ActiveLearningLibtorchNN>("al_nn")),
    _nn_eval(getSurrogateModel<LibtorchANNSurrogate>("nn_evaluator")),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcess>("gp_evaluator")),
    _new_var_samples(_gpry_sampler->getVarSamples()),
    _priors(_gpry_sampler->getPriors()),
    _var_prior(_gpry_sampler->getVarPrior()),
    _check_step(std::numeric_limits<int>::max()),
    _local_comm(_sampler.getLocalComm())
{
  // Check whether the selected sampler is an adaptive sampler or not
  if (!_gpry_sampler)
    paramError("sampler", "The selected sampler is not of type BayesianGPrySampler.");

  // Filling the `likelihoods` vector with the user-provided distributions.
  for (const UserObjectName & name : getParam<std::vector<UserObjectName>>("likelihoods"))
    _likelihoods.push_back(getLikelihoodFunctionByName(name));
  
  // Fetching the sampler characteristics
  _num_samples = _sampler.getNumberOfRows();
  _props = _gpry_sampler->getNumParallelProposals();
  _num_confg_values = _gpry_sampler->getNumberOfConfigValues();
  _num_confg_params = _gpry_sampler->getNumberOfConfigParams();

  _nn_outputs_try.resize(_inputs_all.size());
  _gp_outputs_try.resize(_inputs_all.size());
  _gp_std_try.resize(_inputs_all.size());
}

void
BayesianGPryLearner::setupNNGPData(const std::vector<Real> & log_posterior,
                                   const DenseMatrix<Real> & data_in)
{
  std::vector<Real> tmp;
  tmp.resize(_priors.size());
  for (unsigned int i = 0; i < log_posterior.size(); ++i)
  {
    for (unsigned int j = 0; j < tmp.size(); ++j)
      tmp[j] = data_in(i, j);
    _nn_inputs.push_back(tmp);
    if (log_posterior[i] > 0.0)
    {
      _nn_outputs.push_back(1.0);
      _gp_inputs.push_back(tmp);
      _gp_outputs.push_back(log_posterior[i]);
    }
    else
      _nn_outputs.push_back(0.0);
  }
}

void
BayesianGPryLearner::computeLogPosterior(std::vector<Real> & log_posterior,
                                         const DenseMatrix<Real> & input_matrix)
{
  std::vector<Real> out1(_num_confg_values);
  for (unsigned int i = 0; i < _props; ++i)
  {
    log_posterior[i] = 0.0;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      log_posterior[i] += std::log(_priors[j]->pdf(input_matrix(i, j)));
    for (unsigned int j = 0; j < _num_confg_values; ++j)
      out1[j] = _output_comm[j * _props + i];
    if (_var_prior)
    {
      log_posterior[i] += std::log(_var_prior->pdf(_new_var_samples[i]));
      for (unsigned int j = 0; j < _likelihoods.size(); ++j)
        log_posterior[i] += std::log(_likelihoods[j]->function(out1));
    }
    else
      for (unsigned int j = 0; j < _likelihoods.size(); ++j)
        log_posterior[i] += std::log(_likelihoods[j]->function(out1));
  }
}

void
BayesianGPryLearner::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _t_step)
  {
    _check_step = _t_step;
    return;
  }

  DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      data_in(ss, j) = data[j];
  }
  _local_comm.sum(data_in.get_values());
  _output_comm = _output_value;
  _local_comm.allgather(_output_comm);

  // Compute the log_posterior values
  std::vector<Real> log_posterior(_props);
  computeLogPosterior(log_posterior, data_in);

  if (_t_step > 1)
  {
    setupNNGPData(log_posterior, data_in);

    bool read_nn = false;
    //   if (_t_step > 1)
    //     read_nn = true;
    _al_nn.reTrain(_nn_inputs, _nn_outputs, read_nn);
    _al_gp.reTrain(_gp_inputs, _gp_outputs);

    for (unsigned int i = 0; i < _nn_outputs_try.size(); ++i)
    {
      _nn_outputs_try[i] = _nn_eval.evaluate(_inputs_all[i]);
      _gp_outputs_try[i] = _gp_eval.evaluate(_inputs_all[i], _gp_std_try[i]);
    }

    for (unsigned int i = 0; i < _num_samples; ++i)
      _sorted_indices[i] = i;
  }

  // Track the current step
  _check_step = _t_step;
}

#endif
