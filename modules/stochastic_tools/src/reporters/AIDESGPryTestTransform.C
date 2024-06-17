//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AIDESGPryTestTransform.h"
#include "Sampler.h"
#include "DenseMatrix.h"

registerMooseObject("StochasticToolsApp", AIDESGPryTestTransform);

InputParameters
AIDESGPryTestTransform::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Generic reporter which decides whether or not to accept a proposed "
                             "sample in parallel Markov chain Monte Carlo type of algorithms.");
  params.addParam<ReporterValueName>(
      "outputs_required",
      "outputs_required",
      "Modified value of the model output from this reporter class.");
  params.addParam<ReporterValueName>("inputs", "inputs", "Uncertain inputs to the model.");
  params.addParam<ReporterValueName>("tpm", "tpm", "The transition probability matrix.");
  params.addParam<ReporterValueName>("variance", "variance", "Model variance term.");
  params.addParam<ReporterValueName>(
      "estimated_loglikelihood", "estimated_loglikelihood", "The GP estimated log-likelihood.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluate the trained GP.");
  return params;
}

AIDESGPryTestTransform::AIDESGPryTestTransform(const InputParameters & parameters)
  : GeneralReporter(parameters),
    SurrogateModelInterface(this),
    _outputs_required(declareValue<std::vector<Real>>("outputs_required")),
    _inputs(declareValue<std::vector<std::vector<Real>>>("inputs")),
    _tpm(declareValue<std::vector<Real>>("tpm")),
    _variance(declareValue<std::vector<Real>>("variance")),
    _sampler(getSampler("sampler")),
    _pmcmc(dynamic_cast<const AffineInvariantDESGPry *>(&_sampler)),
    _rnd_vec(_pmcmc->getRandomNumbers()),
    _new_var_samples(_pmcmc->getVarSamples()),
    _priors(_pmcmc->getPriors()),
    _var_prior(_pmcmc->getVarPrior()),
    _gp_eval(getSurrogateModel<GaussianProcess>("gp_evaluator")),
    _check_step(std::numeric_limits<int>::max()),
    _local_comm(_sampler.getLocalComm()),
    _estimated_loglikelihood(declareValue<std::vector<Real>>("estimated_loglikelihood"))
{
  // Check whether the selected sampler is an MCMC sampler or not
  if (!_pmcmc)
    paramError("sampler", "The selected sampler is not of type AffineInvariantDESGPry.");

  // Fetching the sampler characteristics
  _props = _pmcmc->getNumParallelProposals();

  // Resizing the data arrays to transmit to the output file
  _inputs.resize(_props);
  for (unsigned int i = 0; i < _props; ++i)
    _inputs[i].resize(_sampler.getNumberOfCols());
  _outputs_required.resize(_props);
  _outputs_prev.resize(_props);
  _tpm.resize(_props);
  _estimated_loglikelihood.resize(_props);
  _variance.resize(_props);
}

Real
AIDESGPryTestTransform::correctGP(const Real & GPoutput, const Real & trueVariance)
{
  Real correctGP = GPoutput;
  correctGP -= 10.0 * std::log(1.0 / (std::sqrt(2.0 * 1.0 * M_PI)));
  correctGP = correctGP * 1.0 / trueVariance;
  correctGP += 10.0 * std::log(1.0 / (std::sqrt(2.0 * trueVariance * M_PI)));
  return correctGP;
}

void
AIDESGPryTestTransform::computeEvidence(std::vector<Real> & evidence, const DenseMatrix<Real> & input_matrix)
{
  std::vector<Real> tmp;
  tmp.resize(_priors.size());
  Real estimated_evidence;
  Real GP_pred;
//   Real std_dev1;
  //   Real estimated_class;
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
    _estimated_loglikelihood[i] = _gp_eval.evaluate(tmp); // , std_dev1
    GP_pred = _estimated_loglikelihood[i];
    estimated_evidence += (_var_prior) ? correctGP(GP_pred, _new_var_samples[i]) : GP_pred;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      tmp[j] = _data_prev(i, j);
    GP_pred = _gp_eval.evaluate(tmp);
    estimated_evidence -= (_var_prior) ? correctGP(GP_pred, _var_prev[i]) : GP_pred;
    evidence[i] = estimated_evidence;
  }
}

void
AIDESGPryTestTransform::computeTransitionVector(std::vector<Real> & tv,
                                       const std::vector<Real> & evidence)
{
  for (unsigned int i = 0; i < tv.size(); ++i)
    tv[i] = std::exp(std::min(evidence[i], 0.0));
}

void
AIDESGPryTestTransform::nextSamples(std::vector<Real> & req_inputs,
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
    _outputs_required[parallel_index] = _outputs_prev[parallel_index];
  }
}

void
AIDESGPryTestTransform::execute()
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
//   _outputs_required = _output_value;

  // Compute the evidence and transition vectors
  std::vector<Real> evidence(_props);
  if (_t_step > _pmcmc->decisionStep())
  {
    computeEvidence(evidence, data_in);
    _outputs_required = evidence;
    computeTransitionVector(_tpm, evidence);
  }
  else
    _tpm.assign(_props, 1.0);

  // Accept/reject the proposed samples and assign the correct outputs
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
  _outputs_prev = _outputs_required;
  _var_prev = _variance;

  // Track the current step
  _check_step = _t_step;
}
