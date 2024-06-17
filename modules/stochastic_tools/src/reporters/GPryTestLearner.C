//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPryTestLearner.h"
#include "Sampler.h"
#include "DenseMatrix.h"
#include "AdaptiveMonteCarloUtils.h"
#include "StochasticToolsUtils.h"

registerMooseObject("StochasticToolsApp", GPryTestLearner);

InputParameters
GPryTestLearner::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Fast Bayesian inference with the GPry algorithm by El Gammal et al. "
                             "2023: NN and GP training step.");
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

GPryTestLearner::GPryTestLearner(const InputParameters & parameters)
  : GeneralReporter(parameters),
    SurrogateModelInterface(this),
    _output_value(
        getReporterValue<std::vector<Real>>("output_value")), // , REPORTER_MODE_DISTRIBUTED
    _output_comm(declareValue<std::vector<Real>>("output_comm")),
    _sampler(getSampler("sampler")),
    _gpry_sampler(dynamic_cast<const GPryTest *>(&_sampler)),
    _sorted_indices(declareValue<std::vector<unsigned int>>(
        "sorted_indices", std::vector<unsigned int>(_gpry_sampler->getNumParallelProposals(), 0))),
    _inputs_all(_gpry_sampler->getSampleTries()),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcess>("gp_evaluator")),
    // _priors(_gpry_sampler->getPriors()),
    _acquisition_function(declareValue<std::vector<Real>>("acquisition_function")),
    _convergence_value(declareValue<Real>("convergence_value")),
    _check_step(std::numeric_limits<int>::max()),
    _local_comm(_sampler.getLocalComm())
{
  // Check whether the selected sampler is an adaptive sampler or not
  if (!_gpry_sampler)
    paramError("sampler", "The selected sampler is not of type GPryTest.");

  // Fetching the sampler characteristics
  _num_samples = _sampler.getNumberOfRows();
  _props = _gpry_sampler->getNumParallelProposals();

  _gp_outputs_try.resize(_inputs_all.size());
  _gp_std_try.resize(_inputs_all.size());
  _acquisition_function.resize(_props);
  _length_scales.resize(_sampler.getNumberOfCols());
  _eval_outputs_current.resize(_props);

  _output_comm.resize(_props);
}

void
GPryTestLearner::setupNNGPData(const std::vector<Real> & log_posterior,
                                   const DenseMatrix<Real> & data_in)
{
  std::vector<Real> tmp;
  tmp.resize(_sampler.getNumberOfCols());
  for (unsigned int i = 0; i < _props; ++i)
  {
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      tmp[j] = data_in(i, j);

    if (std::exp(log_posterior[i]) > 0.0)
    {
      _gp_inputs.push_back(tmp);
      _gp_outputs.push_back(log_posterior[i]);
    }
  }
}

void
GPryTestLearner::acqWithCorrelations(std::vector<Real> & acq,
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
      computeCorrelation(_inputs_all[j], _inputs_all[ind[0]], correlation);
      acq[j] = acq[j] * correlation;
    }
    Moose::indirectSort(acq.begin(), acq.end(), ind);
    sorted[i + 1] = ind[0];
    acq_new[i + 1] = -acq[ind[0]];
  }
}

void
GPryTestLearner::computeCorrelation(const std::vector<Real> & input1,
                                        const std::vector<Real> & input2,
                                        Real & corr)
{
  corr = 0.0;
  for (unsigned int i = 0; i < input1.size(); ++i)
    corr -= Utility::pow<2>(input1[i] - input2[i]) / (2 * Utility::pow<2>(_length_scales[i]));
  corr = 1.0 - std::exp(corr);
}

void
GPryTestLearner::computeGPOutput2(std::vector<Real> & eval_outputs,
                                      const DenseMatrix<Real> & eval_inputs)
{
  std::vector<Real> tmp;
  tmp.resize(_sampler.getNumberOfCols());
  for (unsigned int i = 0; i < eval_outputs.size(); ++i)
  {
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      tmp[j] = eval_inputs(i, j);
    eval_outputs[i] = _gp_eval.evaluate(tmp);
  }
}

void
GPryTestLearner::computeFunction(const DenseMatrix<Real> & data_in)
{
  for (std::size_t r = 0; r < _props; ++r)
  {
    Real y = 0.0;
    for (std::size_t i = 0; i < _sampler.getNumberOfCols() - 1; ++i)
      y -= (10.0 * Utility::pow<2>(data_in(r, i + 1) - Utility::pow<2>(data_in(r, i))) +
            Utility::pow<2>(1 - data_in(r, i))) /
           20.0;
    _output_comm[r] = y;
  }
}

void
GPryTestLearner::execute()
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
//   _output_comm = _output_value;
//   _local_comm.allgather(_output_comm);
  computeFunction(data_in);

//   for (unsigned int ii = 0; ii < _output_comm.size(); ++ii)
//   {
//     std::cout << "Input " << data_in(ii, 0) << "," << data_in(ii, 1) << "," << data_in(ii, 2) << ","
//               << data_in(ii, 3) << "," << data_in(ii, 4) << std::endl;
//     std::cout << "Output " << _output_comm[ii] << std::endl;
//   }

  if (_t_step > 1)
  {
    setupNNGPData(_output_comm, data_in);
    // for (unsigned int ii = 0; ii < _gp_inputs.size(); ++ii)
    // {
    //   std::cout << Moose::stringify(_gp_inputs[ii]) << std::endl; //  data_in(ii, 0)
    // }
    // std::cout << "Output " << Moose::stringify(_gp_outputs) << std::endl;
    _convergence_value = 0.0;
    if (_t_step > 2)
    {
      computeGPOutput2(_eval_outputs_current, data_in);
      for (unsigned int ii = 0; ii < _output_comm.size(); ++ii)
        _convergence_value += Utility::pow<2>(_output_comm[ii] - _eval_outputs_current[ii]);
    }
    _convergence_value = std::sqrt(_convergence_value); // / (_output_comm.size());
    _al_gp.reTrain(_gp_inputs, _gp_outputs);

    _al_gp.getLengthScales(_length_scales);

    std::cout << "Num training points " << Moose::stringify(_gp_outputs.size()) << std::endl;

    std::vector<Real> tmp;
    tmp.resize(_sampler.getNumberOfCols());
    for (unsigned int i = 0; i < _gp_outputs_try.size(); ++i)
    {
      for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
        tmp[j] = _inputs_all[i][j];
      _gp_outputs_try[i] = _gp_eval.evaluate(tmp, _gp_std_try[i]);
    }

    // Paper acquisition function
    Real psi = std::pow(_sampler.getNumberOfCols(), -0.85);
    std::vector<Real> acq, acq_new;
    acq.resize(_inputs_all.size());
    acq_new.resize(_inputs_all.size());
    Real gp_mean;
    Real gp_std;
    for (unsigned int i = 0; i < _inputs_all.size(); ++i)
    {
      gp_mean = _gp_outputs_try[i];
      gp_std = _gp_std_try[i];
      acq[i] = -std::exp(2.0 * psi * gp_mean) * (std::exp(gp_std) - 1.0);
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
  }

  // Track the current step
  _check_step = _t_step;
}
