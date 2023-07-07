//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "IndependentMetropolisHastingsDecision.h"

registerMooseObjectAliased("StochasticToolsApp",
                           IndependentMetropolisHastingsDecision,
                           "IndependentMHDecision");

InputParameters
IndependentMetropolisHastingsDecision::validParams()
{
  InputParameters params = ParallelMarkovChainMonteCarloDecision::validParams();
  params.addClassDescription("Perform decision making for independent Metropolis-Hastings MCMC.");
  params.addParam<ReporterValueName>(
      "seed_input", "seed_input", "The seed vector input for proposing new samples.");
  return params;
}

IndependentMetropolisHastingsDecision::IndependentMetropolisHastingsDecision(
    const InputParameters & parameters)
  : ParallelMarkovChainMonteCarloDecision(parameters),
    _igmh(dynamic_cast<const IndependentGaussianMetropolisHastings *>(&_sampler)),
    _seed_input(declareValue<std::vector<Real>>("seed_input"))
{
  // Check whether the selected sampler is a stretch sampler or not
  if (!_igmh)
    paramError("sampler",
               "The selected sampler is not of type IndependentGaussianMetropolisHastings.");

  _seed_outputs.resize(_num_confg);
}

void
IndependentMetropolisHastingsDecision::computeEvidence(std::vector<Real> & evidence,
                                                       DenseMatrix<Real> & inputs_matrix)
{
  std::vector<Real> out1(_num_confg);
  for (unsigned int i = 0; i < evidence.size(); ++i)
  {
    evidence[i] = 0.0;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      evidence[i] += (std::log(_priors[j]->pdf(inputs_matrix(i, j))) -
                      std::log(_priors[j]->pdf(_seed_input[j])));
    for (unsigned int j = 0; j < _num_confg; ++j)
      out1[j] = _outputs_required[j * _props + i];
    for (unsigned int j = 0; j < _likelihoods.size(); ++j)
      evidence[i] += (_likelihoods[j]->function(out1) - _likelihoods[j]->function(_seed_outputs));
  }
}

void
IndependentMetropolisHastingsDecision::computeTransitionVector(std::vector<Real> & tv,
                                                               std::vector<Real> & evidence)
{
  for (unsigned int i = 0; i < tv.size(); ++i)
    tv[i] = (1.0 / tv.size()) * std::exp(std::min(evidence[i], 0.0));
  _tpm_modified = tv;
  _tpm_modified.push_back((1.0 - std::accumulate(tv.begin(), tv.end(), 0.0)));
  _outputs_sto = _outputs_required;
}

void
IndependentMetropolisHastingsDecision::nextSamples(std::vector<Real> & req_inputs,
                                                   DenseMatrix<Real> & inputs_matrix,
                                                   const std::vector<Real> & /*tv*/,
                                                   const unsigned int & parallel_index)
{
  unsigned int index =
      AdaptiveMonteCarloUtils::weightedResample(_tpm_modified, _rnd_vec[parallel_index]);
  if (index < _props)
  {
    for (unsigned int k = 0; k < _sampler.getNumberOfCols() - 1; ++k)
      req_inputs[k] = inputs_matrix(index, k);
    for (unsigned int k = 0; k < _num_confg; ++k)
      _outputs_required[k * _props + parallel_index] = _outputs_sto[k * _props + index];
  }
  else
  {
    req_inputs = _seed_input;
    for (unsigned int k = 0; k < _num_confg; ++k)
      _outputs_required[k * _props + parallel_index] = _seed_outputs[k];
  }
}

void
IndependentMetropolisHastingsDecision::nextSeeds()
{
  _seed_input = _inputs[_props - 1];
  for (unsigned int k = 0; k < _num_confg; ++k)
    _seed_outputs[k] = _outputs_required[(k + 1) * _props - 1];
}
