//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IndependentMHDecision.h"

registerMooseObject("StochasticToolsApp", IndependentMHDecision);

InputParameters
IndependentMHDecision::validParams()
{
  InputParameters params = PMCMCDecision::validParams();
  params.addClassDescription("Perform decision making for independent Metropolis-Hastings MCMC.");
  params.addParam<ReporterValueName>(
      "seed_input", "seed_input", "The seed vector input for proposing new samples.");
  return params;
}

IndependentMHDecision::IndependentMHDecision(const InputParameters & parameters)
  : PMCMCDecision(parameters),
    _igmh(dynamic_cast<const IndependentGaussianMH *>(&_sampler)),
    _seed_input(declareValue<std::vector<Real>>("seed_input"))
{
  // Check whether the selected sampler is a M-H sampler or not
  if (!_igmh)
    paramError("sampler", "The selected sampler is not of type IndependentGaussianMH.");

  _seed_outputs.resize(_num_confg_values);
  _tpm_modified.assign(_props + 1, 1.0 / (_props + 1));
}

void
IndependentMHDecision::computeEvidence(std::vector<Real> & evidence,
                                       const DenseMatrix<Real> & input_matrix)
{
  std::vector<Real> out(_num_confg_values);
  for (unsigned int i = 0; i < evidence.size(); ++i)
  {
    evidence[i] = 0.0;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      evidence[i] += (std::log(_priors[j]->pdf(input_matrix(i, j))) -
                      std::log(_priors[j]->pdf(_seed_input[j])));
    for (unsigned int j = 0; j < _num_confg_values; ++j)
      out[j] = _outputs_required[j * _props + i];
    for (unsigned int j = 0; j < _likelihoods.size(); ++j)
      evidence[i] += (_likelihoods[j]->function(out) - _likelihoods[j]->function(_seed_outputs));
  }
}

void
IndependentMHDecision::computeTransitionVector(std::vector<Real> & tv,
                                               const std::vector<Real> & evidence)
{
  for (unsigned int i = 0; i < tv.size(); ++i)
    tv[i] = (1.0 / tv.size()) * std::exp(std::min(evidence[i], 0.0));
  _tpm_modified = tv;
  _tpm_modified.push_back((1.0 - std::accumulate(tv.begin(), tv.end(), 0.0)));
  _outputs_sto = _outputs_required;
}

void
IndependentMHDecision::nextSamples(std::vector<Real> & req_inputs,
                                   DenseMatrix<Real> & input_matrix,
                                   const std::vector<Real> & /*tv*/,
                                   const unsigned int & parallel_index)
{
  const bool value = (_tpm_modified[0] == 1.0 / (_props + 1));
  if (!value)
  {
    unsigned int index =
        AdaptiveMonteCarloUtils::weightedResample(_tpm_modified, _rnd_vec[parallel_index]);
    if (index < _props)
    {
      for (unsigned int k = 0; k < _sampler.getNumberOfCols() - _num_confg_params; ++k)
        req_inputs[k] = input_matrix(index, k);
      for (unsigned int k = 0; k < _num_confg_values; ++k)
        _outputs_required[k * _props + parallel_index] = _outputs_sto[k * _props + index];
    }
    else
    {
      req_inputs = _seed_input;
      for (unsigned int k = 0; k < _num_confg_values; ++k)
        _outputs_required[k * _props + parallel_index] = _seed_outputs[k];
    }
  }
  else
  {
    for (unsigned int k = 0; k < _sampler.getNumberOfCols() - _num_confg_params; ++k)
      req_inputs[k] = input_matrix(parallel_index, k);
    _variance[parallel_index] = _new_var_samples[parallel_index];
  }
}

void
IndependentMHDecision::nextSeeds()
{
  _seed_input = _inputs[_props - 1];
  for (unsigned int k = 0; k < _num_confg_values; ++k)
    _seed_outputs[k] = _outputs_required[(k + 1) * _props - 1];
}
