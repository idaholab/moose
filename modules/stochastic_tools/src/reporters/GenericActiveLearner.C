//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GenericActiveLearner.h"
#include "Sampler.h"
#include "DenseMatrix.h"
#include "AdaptiveMonteCarloUtils.h"
#include "StochasticToolsUtils.h"

registerMooseObject("StochasticToolsApp", GenericActiveLearner);

InputParameters
GenericActiveLearner::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Generic active learning using GP.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>(
      "output_comm", "output_comm", "Modified value of the model output from this reporter class.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<UserObjectName>("al_gp", "Active learning GP trainer.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluate the trained GP.");
  params.addParam<ReporterValueName>(
      "sorted_indices",
      "sorted_indices",
      "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addParam<ReporterValueName>(
      "acquisition_function",
      "acquisition_function",
      "The values of the acquistion function in the current iteration.");
  params.addParam<ReporterValueName>("convergence_value",
                                     "convergence_value",
                                     "Value to measure convergence of the GPry algorithm.");
  return params;
}

GenericActiveLearner::GenericActiveLearner(const InputParameters & parameters)
  : GeneralReporter(parameters),
    SurrogateModelInterface(this),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _output_comm(declareValue<std::vector<Real>>("output_comm")),
    _sampler(getSampler("sampler")),
    _al_sampler(dynamic_cast<const GenericActiveLearningSampler *>(&_sampler)),
    _sorted_indices(declareValue<std::vector<unsigned int>>(
        "sorted_indices", std::vector<unsigned int>(_al_sampler->getNumParallelProposals(), 0))),
    _inputs_all(_al_sampler->getSampleTries()),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcessSurrogate>("gp_evaluator")),
    _acquisition_function(declareValue<std::vector<Real>>("acquisition_function")),
    _convergence_value(declareValue<Real>("convergence_value")),
    _check_step(std::numeric_limits<int>::max()),
    _local_comm(_sampler.getLocalComm())
{
  // Check whether the selected sampler is an adaptive sampler or not
  if (!_al_sampler)
    paramError("sampler", "The selected sampler is not of type BayesianGPrySampler.");
  
  // Fetching the sampler characteristics
  _num_samples = _sampler.getNumberOfRows();
  _n_dim = _sampler.getNumberOfCols();
  _props = _al_sampler->getNumParallelProposals();

  _gp_outputs_try.resize(_inputs_all.size());
  _gp_std_try.resize(_inputs_all.size());
  _acquisition_function.resize(_props); //
  _length_scales.resize(_n_dim);

  _eval_points = 10000;
  // _eval_outputs_current.resize(_eval_points);
  // _eval_outputs_previous.resize(_eval_points);
  _eval_outputs_current.resize(_props);
  _eval_outputs_previous.resize(_props);
  _eval_inputs.resize(_eval_points, std::vector<Real>(_n_dim, 0.0));
}

// void
// GenericActiveLearner::setupCovariance(UserObjectName covar_name)
// {
//   if (_gp_handler.getCovarFunctionPtr() != nullptr)
//     ::mooseError("Attempting to redefine covariance function using setupCovariance.");
//   _gp_handler.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
// }

void
GenericActiveLearner::setupGPData(const std::vector<Real> & log_posterior,
                                   const DenseMatrix<Real> & data_in)
{
  std::vector<Real> tmp;
  tmp.resize(_n_dim);
  for (unsigned int i = 0; i < log_posterior.size(); ++i)
  {
    for (unsigned int j = 0; j < _n_dim; ++j)
      tmp[j] = data_in(i, j);
    _gp_inputs.push_back(tmp);
    _gp_outputs.push_back(log_posterior[i]);
  }
}

void
GenericActiveLearner::acqWithCorrelations(std::vector<Real> & acq,
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
      // computeCorrelation(_inputs_all[j], _inputs_all[ind[0]], correlation);
      computeCorrelation(j, ind[0], correlation);
      acq[j] = acq[j] * correlation;
    }
    Moose::indirectSort(acq.begin(), acq.end(), ind);
    sorted[i + 1] = ind[0];
    acq_new[i + 1] = -acq[ind[0]];
  }
}

void
GenericActiveLearner::computeCorrelation(const unsigned int & ind1,
                                        const unsigned int & ind2,
                                        Real & corr)
{
  corr = 0.0;
  for (unsigned int i = 0; i < _n_dim; ++i)
    corr -= Utility::pow<2>(_inputs_all[ind1][i] - _inputs_all[ind2][i]) /
            (2.0 * Utility::pow<2>(_length_scales[i]));
  corr = 1.0 - std::exp(corr); // 1.0 - std::exp(corr);
}

void
GenericActiveLearner::computeGPOutput(std::vector<Real> & eval_outputs,
                                     const std::vector<std::vector<Real>> & eval_inputs)
{
  for (unsigned int i = 0; i < eval_outputs.size(); ++i)
    eval_outputs[i] = _gp_eval.evaluate(eval_inputs[i]);
}

void
GenericActiveLearner::computeGPOutput2(std::vector<Real> & eval_outputs,
                                      const DenseMatrix<Real> & eval_inputs)
{
  std::vector<Real> tmp;
  tmp.resize(_n_dim);
  for (unsigned int i = 0; i < eval_outputs.size(); ++i)
  {
    for (unsigned int j = 0; j < _n_dim; ++j)
      tmp[j] = eval_inputs(i, j);
    eval_outputs[i] = _gp_eval.evaluate(tmp);
  }
}

void
GenericActiveLearner::computeDistance(const std::vector<Real> & current_input,
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
GenericActiveLearner::execute()
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

  if (_t_step > 1)
  {
    setupGPData(_output_comm, data_in);
    _convergence_value = 0.0;
    unsigned int num = 0.0;
    if (_t_step > 2)
    {
      computeGPOutput2(_eval_outputs_current, data_in);
      for (unsigned int ii = 0; ii < _output_comm.size(); ++ii)
        _convergence_value += Utility::pow<2>(_output_comm[ii] - _eval_outputs_current[ii]);
    }
    _convergence_value = std::sqrt(_convergence_value) / _output_comm.size();
    // std::cout << "Num train points: " << _gp_outputs.size() << std::endl;
    _al_gp.reTrain(_gp_inputs, _gp_outputs);

    _al_gp.getLengthScales(_length_scales);

    std::vector<Real> tmp;
    tmp.resize(_n_dim);
    for (unsigned int i = 0; i < _gp_outputs_try.size(); ++i)
    {
      for (unsigned int j = 0; j < _n_dim; ++j)
        tmp[j] = _inputs_all[i][j];
      _gp_outputs_try[i] = _gp_eval.evaluate(tmp, _gp_std_try[i]);
    }

    // Expected improvement in global fit
    std::vector<Real> acq, acq_new;
    acq.resize(_inputs_all.size());
    acq_new.resize(_inputs_all.size());
    Real gp_mean;
    Real gp_std;
    unsigned int ref_ind;
    for (unsigned int i = 0; i < _inputs_all.size(); ++i)
    {
      gp_mean = _gp_outputs_try[i];
      gp_std = _gp_std_try[i];
      for (unsigned int j = 0; j < _n_dim; ++j)
        tmp[j] = _inputs_all[i][j];
      computeDistance(tmp, ref_ind);
      acq[i] = -(std::pow((gp_mean - _gp_outputs[ref_ind]), 2) + std::pow(gp_std, 2));
    }

    std::vector<unsigned int> tmp_indices;
    tmp_indices.resize(_inputs_all.size());
    acqWithCorrelations(acq, tmp_indices, acq_new);
    for (unsigned int i = 0; i < _props; ++i)
    {
      _sorted_indices[i] = tmp_indices[i];
      _acquisition_function[i] = acq_new[i];
    }
    
    std::cout << "_convergence_value " << _convergence_value << std::endl;
  }
  else
  {
    for (unsigned int i = 0; i < _props; ++i)
      _sorted_indices[i] = i;
    // for (unsigned int i = 0; i < _eval_points; ++i)
    // {
    //   fillVector(_eval_inputs[i]);
    //   if (_var_prior)
    //     _eval_inputs[i][_priors.size()] = _var_prior->quantile(_sampler.getRand(_seed));
    // }
  }

  // Track the current step
  _check_step = _t_step;
}
