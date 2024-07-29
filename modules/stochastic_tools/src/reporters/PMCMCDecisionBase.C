//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PMCMCDecisionBase.h"

registerMooseObject("StochasticToolsApp", PMCMCDecisionBase);

InputParameters
PMCMCDecisionBase::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Generic base reporter which decides whether or not to accept a proposed "
                             "sample in parallel Markov chain Monte Carlo type of algorithms.");
  params.addParam<ReporterValueName>("inputs", "inputs", "Uncertain inputs to the model.");
  params.addParam<ReporterValueName>("tpm", "tpm", "The transition probability matrix.");
  params.addParam<ReporterValueName>("variance", "variance", "Model variance term.");
  params.addParam<ReporterValueName>(
      "noise", "noise", "Model noise term to pass to Likelihoods object.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  return params;
}

PMCMCDecisionBase::PMCMCDecisionBase(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _inputs(declareValue<std::vector<std::vector<Real>>>("inputs")),
    _tpm(declareValue<std::vector<Real>>("tpm")),
    _variance(declareValue<std::vector<Real>>("variance")),
    _noise(declareValue<Real>("noise")),
    _sampler(getSampler("sampler")),
    _pmcmc(dynamic_cast<const PMCMCBase *>(&_sampler)),
    _rnd_vec(_pmcmc->getRandomNumbers()),
    _new_var_samples(_pmcmc->getVarSamples()),
    _priors(_pmcmc->getPriors()),
    _var_prior(_pmcmc->getVarPrior()),
    _local_comm(_sampler.getLocalComm()),
    _check_step(std::numeric_limits<int>::max())
{
  // Fetching the sampler characteristics
  _props = _pmcmc->getNumParallelProposals();
  _num_confg_values = _pmcmc->getNumberOfConfigValues();
  _num_confg_params = _pmcmc->getNumberOfConfigParams();

  // Resizing the data arrays to transmit to the output file
  _inputs.resize(_props);
  for (unsigned int i = 0; i < _props; ++i)
    _inputs[i].resize(_sampler.getNumberOfCols() - _num_confg_params);
  _tpm.resize(_props);
  _variance.resize(_props);
}
