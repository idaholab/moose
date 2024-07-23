//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

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
  params.addRequiredParam<ReporterName>("output_value1",
                                        "Value of the model output1 from the SubApp.");
  params.addParam<ReporterValueName>(
      "output_comm", "output_comm", "Modified value of the model output from this reporter class.");
  params.addParam<ReporterValueName>(
      "output_comm1", "output_comm1", "Modified value of the model output from this reporter class.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<UserObjectName>("al_gp", "Active learning GP trainer.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluate the trained GP.");
  params.addParam<ReporterValueName>(
      "sorted_indices",
      "sorted_indices",
      "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addParam<ReporterValueName>(
      "noise", "noise", "Model noise term to pass to Likelihoods object.");
  params.addParam<ReporterValueName>(
      "acquisition_function",
      "acquisition_function",
      "The values of the acquistion function in the current iteration.");
  params.addParam<ReporterValueName>("convergence_value",
                                     "convergence_value",
                                     "Value to measure convergence of the GPry algorithm.");
  params.addRequiredParam<std::vector<UserObjectName>>("likelihoods", "Names of likelihoods.");
  return params;
}

BayesianGPryLearner::BayesianGPryLearner(const InputParameters & parameters)
  : GeneralReporter(parameters),
    LikelihoodInterface(parameters),
    // CovarianceInterface(parameters),
    SurrogateModelInterface(this),
    // _gp_handler(declareModelData<StochasticTools::GaussianProcessHandler>("_gp_handler")),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _output_comm(declareValue<std::vector<Real>>("output_comm")),
    _output_value1(getReporterValue<std::vector<Real>>("output_value1", REPORTER_MODE_DISTRIBUTED)),
    _output_comm1(declareValue<std::vector<Real>>("output_comm1")),
    _sampler(getSampler("sampler")),
    _gpry_sampler(dynamic_cast<const BayesianActiveLearningSampler *>(&_sampler)),
    _sorted_indices(declareValue<std::vector<unsigned int>>(
        "sorted_indices",
        std::vector<unsigned int>(_gpry_sampler->getNumParallelProposals(),
                                  0))),
    _inputs_all(_gpry_sampler->getSampleTries()),
    _var_all(_gpry_sampler->getVarSampleTries()),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcessSurrogate>("gp_evaluator")),
    _new_var_samples(_gpry_sampler->getVarSamples()),
    _priors(_gpry_sampler->getPriors()),
    _var_prior(_gpry_sampler->getVarPrior()),
    _noise(declareValue<Real>("noise")),
    _acquisition_function(declareValue<std::vector<Real>>("acquisition_function")),
    _convergence_value(declareValue<Real>("convergence_value")),
    _check_step(std::numeric_limits<int>::max()),
    _local_comm(_sampler.getLocalComm())
{
  // Check whether the selected sampler is an adaptive sampler or not
  if (!_gpry_sampler)
    paramError("sampler", "The selected sampler is not of type BayesianActiveLearningSampler.");
  
  // Filling the `likelihoods` vector with the user-provided distributions.
  for (const UserObjectName & name : getParam<std::vector<UserObjectName>>("likelihoods"))
    _likelihoods.push_back(getLikelihoodFunctionByName(name));
  
  // Fetching the sampler characteristics
  _num_samples = _sampler.getNumberOfRows();
  _props = _gpry_sampler->getNumParallelProposals();
  _num_confg_values = _gpry_sampler->getNumberOfConfigValues();
  _num_confg_params = _gpry_sampler->getNumberOfConfigParams();

  _gp_outputs_try.resize(_inputs_all.size());
  _gp_std_try.resize(_inputs_all.size());
  _acquisition_function.resize(_props); // 
  _length_scales.resize(_priors.size());

  _eval_points = 10000;
  // _eval_outputs_current.resize(_eval_points);
  // _eval_outputs_previous.resize(_eval_points);
  _eval_outputs_current.resize(_props);
  _eval_outputs_previous.resize(_props);
  if (_var_prior)
    _eval_inputs.resize(_eval_points, std::vector<Real>(_priors.size() + 1, 0.0));
  else
    _eval_inputs.resize(_eval_points, std::vector<Real>(_priors.size(), 0.0));
}

// void
// BayesianGPryLearner::setupCovariance(UserObjectName covar_name)
// {
//   if (_gp_handler.getCovarFunctionPtr() != nullptr)
//     ::mooseError("Attempting to redefine covariance function using setupCovariance.");
//   _gp_handler.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
// }

void
BayesianGPryLearner::setupNNGPData(const std::vector<Real> & log_posterior,
                                   const DenseMatrix<Real> & data_in)
{
  std::vector<Real> tmp;
  if (_var_prior)
    tmp.resize(_priors.size() + 1);
  else
    tmp.resize(_priors.size());
  for (unsigned int i = 0; i < log_posterior.size(); ++i)
  {
    for (unsigned int j = 0; j < _priors.size(); ++j)
      tmp[j] = data_in(i, j);
    if (_var_prior)
      tmp[_priors.size()] = _new_var_samples[i];
    if (log_posterior[i] == log_posterior[i]) // std::exp(log_posterior[i]) > 0.0
    {
      _gp_inputs.push_back(tmp);
      _gp_outputs.push_back(log_posterior[i]);
    }
    // _gp_inputs.push_back(tmp);
    // _gp_outputs.push_back(log_posterior[i]);
  }
}

void
BayesianGPryLearner::computeLogPosterior(std::vector<Real> & log_posterior,
                                         const DenseMatrix<Real> & input_matrix)
{
  std::vector<Real> out1(_num_confg_values);
  std::vector<Real> out11(_num_confg_values);
  for (unsigned int i = 0; i < _props; ++i)
  {
    log_posterior[i] = 0.0;
    // for (unsigned int j = 0; j < _priors.size(); ++j)
    //   log_posterior[i] += std::log(_priors[j]->pdf(input_matrix(i, j)));
    for (unsigned int j = 0; j < _num_confg_values; ++j)
    {
      out1[j] = _output_comm[j * _props + i];
      out11[j] = _output_comm1[j * _props + i];
    }
    if (_var_prior)
    {
      // log_posterior[i] += std::log(_var_prior->pdf(_new_var_samples[i]));
      _noise = std::sqrt(_new_var_samples[i]);
      log_posterior[i] +=
          _likelihoods[0]->function(out1); // std::log(_likelihoods[0]->function(out1)); //
      log_posterior[i] +=
          _likelihoods[1]->function(out11); // std::log(_likelihoods[1]->function(out11)); //
    }
    else
    {
      log_posterior[i] +=
          _likelihoods[0]->function(out1); // std::log(_likelihoods[0]->function(out1)); //
      log_posterior[i] +=
          _likelihoods[1]->function(out11); // std::log(_likelihoods[1]->function(out11)); //
    }
  }
}

void
BayesianGPryLearner::acqWithCorrelations(std::vector<Real> & acq,
                                         std::vector<unsigned int> & sorted,
                                         std::vector<Real> & acq_new)
{
  Real correlation = 0.0;
  std::vector<size_t> ind;
  Moose::indirectSort(acq.begin(), acq.end(), ind);
  sorted[0] = ind[0];
  acq_new[0] = -acq[ind[0]];
  for (unsigned int i = 0; i < _inputs_all.size() - 1; ++i)
  {
    for (unsigned int j = 0; j < _inputs_all.size(); ++j)
    {
      computeCorrelation(j, ind[0], correlation);
      acq[j] = acq[j] * correlation;
    }
    Moose::indirectSort(acq.begin(), acq.end(), ind);
    sorted[i + 1] = ind[0];
    acq_new[i + 1] = -acq[ind[0]];
  }
}

// void
// BayesianGPryLearner::computeCorrelation(const std::vector<Real> & input1,
//                                         const std::vector<Real> & input2,
//                                         Real & corr)
// {
//   corr = 0.0;
//   for (unsigned int i = 0; i < input1.size(); ++i)
//     corr -= Utility::pow<2>(input1[i] - input2[i]) / (2 * Utility::pow<2>(_length_scales[i]));
//   corr = 1.0 - std::exp(corr); // 1.0 - std::exp(corr);
// }

void
BayesianGPryLearner::computeCorrelation(const unsigned int & ind1,
                                        const unsigned int & ind2,
                                        Real & corr)
{
  corr = 0.0;
  for (unsigned int i = 0; i < _priors.size(); ++i)
    corr -= Utility::pow<2>(_inputs_all[ind1][i] - _inputs_all[ind2][i]) /
            (2.0 * Utility::pow<2>(_length_scales[i]));
  if (_var_prior)
    corr -= Utility::pow<2>(_var_all[ind1] - _var_all[ind2]) /
            (2.0 * Utility::pow<2>(_length_scales[_priors.size()]));
  corr = 1.0 - std::exp(corr); // 1.0 - std::exp(corr);
}

void
BayesianGPryLearner::computeGPOutput(std::vector<Real> & eval_outputs,
                                     const std::vector<std::vector<Real>> & eval_inputs)
{
  for (unsigned int i = 0; i < eval_outputs.size(); ++i)
    eval_outputs[i] = _gp_eval.evaluate(eval_inputs[i]);
}

void
BayesianGPryLearner::computeGPOutput2(std::vector<Real> & eval_outputs,
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
BayesianGPryLearner::fillVector(std::vector<Real> & vector)
{
  for (unsigned int i = 0; i < _priors.size(); ++i)
    vector[i] = _priors[i]->quantile(_sampler.getRand(_seed));
}

void
BayesianGPryLearner::computeDistance(const std::vector<Real> & current_input,
                                     unsigned int & req_index)
{
  Real ref_distance = 1e10;
  Real distance;
  req_index = 0;
  for (unsigned int i = 0; i < _gp_outputs.size(); ++i)
  {
    distance = 0.0;
    for (unsigned int j = 0; j < current_input.size(); ++j)
      distance += std::abs(current_input[j] - _gp_inputs[i][j]);
    if (distance <= ref_distance)
    {
      ref_distance = distance;
      req_index = i;
    }
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
  _output_comm1 = _output_value1;
  _local_comm.allgather(_output_comm1);

  // Compute the log_posterior values
  std::vector<Real> log_posterior(_props);
  computeLogPosterior(log_posterior, data_in);

  if (_t_step > 1)
  {
    setupNNGPData(log_posterior, data_in);
    std::cout << "_gp_outputs " << Moose::stringify(_gp_outputs) << std::endl;
    _convergence_value = 0.0;
    unsigned int num = 0.0;
    if (_t_step > 2)
    {
      computeGPOutput2(_eval_outputs_current, data_in);
      for (unsigned int ii = 0; ii < log_posterior.size(); ++ii)
      {
        // if (std::exp(log_posterior[ii]) > 0.0)
        // {
        //   ++num;
        //   _convergence_value += Utility::pow<2>(log_posterior[ii] - _eval_outputs_current[ii]);
        // }
        if (log_posterior[ii] == log_posterior[ii])
          _convergence_value += Utility::pow<2>(log_posterior[ii] - _eval_outputs_current[ii]);
      }
    }
    _convergence_value = std::sqrt(_convergence_value); // / num; // / (_output_comm.size());
    _al_gp.reTrain(_gp_inputs, _gp_outputs);

    _al_gp.getLengthScales(_length_scales);
    // std::cout << "_length_scales " << Moose::stringify(_length_scales) << std::endl;

    // std::cout << "Num training points " << Moose::stringify(_gp_outputs.size()) << std::endl;

    std::vector<Real> tmp;
    if (_var_prior)
      tmp.resize(_priors.size() + 1);
    else
      tmp.resize(_priors.size());
    for (unsigned int i = 0; i < _gp_outputs_try.size(); ++i)
    {
      for (unsigned int j = 0; j < _priors.size(); ++j)
        tmp[j] = _inputs_all[i][j];
      if (_var_prior)
        tmp[_priors.size()] = _var_all[i];
      _gp_outputs_try[i] = _gp_eval.evaluate(tmp, _gp_std_try[i]);
    }

    // std::cout << "_gp_outputs " << Moose::stringify(_gp_outputs) << std::endl;

    // Paper acquisition function
    Real psi = std::pow(_priors.size(), -0.85);
    std::vector<Real> acq, acq_new;
    acq.resize(_inputs_all.size());
    acq_new.resize(_inputs_all.size());
    Real gp_mean;
    Real gp_std;
    for (unsigned int i = 0; i < _inputs_all.size(); ++i)
    {
      gp_mean = _gp_outputs_try[i];
      gp_std = _gp_std_try[i];
      // std::cout << gp_mean << " " << gp_std << std::endl;
      acq[i] = -std::exp(2.0 * psi * gp_mean) * (std::exp(gp_std) - 1.0);
      if (std::isinf(acq[i]))
        acq[i] = -1e8;
      // acq[i] = -2.0 * psi * gp_mean - gp_std;
    }
    // std::cout << "acq " << Moose::stringify(acq) << std::endl;

    // Expected improvement in global fit
    // std::vector<Real> acq, acq_new;
    // acq.resize(_inputs_all.size());
    // acq_new.resize(_inputs_all.size());
    // Real gp_mean;
    // Real gp_std;
    // unsigned int ref_ind;
    // for (unsigned int i = 0; i < _inputs_all.size(); ++i)
    // {
    //   gp_mean = _gp_outputs_try[i];
    //   gp_std = _gp_std_try[i];
    //   for (unsigned int j = 0; j < _priors.size(); ++j)
    //     tmp[j] = _inputs_all[i][j];
    //   if (_var_prior)
    //     tmp[_priors.size()] = _var_all[i];
    //   computeDistance(tmp, ref_ind);
    //   acq[i] = -(std::pow((gp_mean - _gp_outputs[ref_ind]), 2) + std::pow(gp_std, 2));
    // }

    // std::vector<size_t> ind;
    // Moose::indirectSort(acq.begin(), acq.end(), ind);
    // for (unsigned int i = 0; i < _props; ++i)
    // {
    //   _sorted_indices[i] = ind[i];
    //   _acquisition_function[i] = -acq[ind[i]];
    // }

    std::vector<unsigned int> tmp_indices;
    tmp_indices.resize(_inputs_all.size());
    acqWithCorrelations(acq, tmp_indices, acq_new);
    for (unsigned int i = 0; i < _props; ++i)
    {
      _sorted_indices[i] = tmp_indices[i];
      _acquisition_function[i] = acq_new[i];
    }
    
    // computeGPOutput(_eval_outputs_current, _eval_inputs);
    // if (_t_step > 1)
    // {
    //   _convergence_value = 0.0;
    //   for (unsigned int ii = 0; ii < _eval_points; ++ii)
    //     _convergence_value +=
    //         std::abs(_eval_outputs_previous[ii] - _eval_outputs_current[ii]) /
    //         (_eval_points); // std::log(_eval_inputs_density[ii])
    // }
    // _eval_outputs_previous = _eval_outputs_current;
    std::cout << "_convergence_value " << _convergence_value << std::endl;
  }
  else
  {
    for (unsigned int i = 0; i < _props; ++i)
      _sorted_indices[i] = i;
    for (unsigned int i = 0; i < _eval_points; ++i)
    {
      fillVector(_eval_inputs[i]);
      if (_var_prior)
        _eval_inputs[i][_priors.size()] = _var_prior->quantile(_sampler.getRand(_seed));
    }
  }

  // Track the current step
  _check_step = _t_step;
}
