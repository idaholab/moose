//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ParallelMarkovChainMonteCarloDecision.h"
#include "Sampler.h"
#include "DenseMatrix.h"
#include "AdaptiveMonteCarloUtils.h"
#include "StochasticToolsUtils.h"

registerMooseObjectAliased("StochasticToolsApp", ParallelMarkovChainMonteCarloDecision, "PMCMCDecision");

InputParameters
ParallelMarkovChainMonteCarloDecision::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Generic reporter which decides whether or not to accept a proposed "
                             "sample in parallel Markov chain Monte Carlo type of algorithms.");
  params.addRequiredParam<ReporterName>("seed_inputs",
                                        "Seed input values for proposing the next set of samples.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addParam<ReporterValueName>("inputs", "inputs", "Uncertain inputs to the model.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<std::vector<LikelihoodName>>("likelihoods", "Names of the likelihoods.");
  params.addRequiredParam<std::vector<DistributionName>>("prior distributions", "The prior distributions of the parameters to be calibrated.");
  return params;
}

ParallelMarkovChainMonteCarloDecision::ParallelMarkovChainMonteCarloDecision(const InputParameters & parameters)
  : GeneralReporter(parameters),
    LikelihoodInterface(this),
    DistributionInterface(this),
    _seed_inputs(declareValue<std::vector<Real>>("seed_inputs")),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _inputs(declareValue<std::vector<std::vector<Real>>>("inputs")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _sampler(getSampler("sampler")),
    _pmcmc(dynamic_cast<const ParallelMarkovChainMonteCarloBase *>(&_sampler)),
    _check_step(std::numeric_limits<int>::max())
{
  
  for (const LikelihoodName & name : getParam<std::vector<LikelihoodName>>("likelihoods"))
    _likelihoods.push_back(&getLikelihoodByName(name));
  
  // Filling the `priors` vector with the user-provided distributions.
  for (const DistributionName & name : getParam<std::vector<DistributionName>>("distributions"))
    _priors.push_back(&getDistributionByName(name));

  // Check whether the selected sampler is an MCMC sampler or not
  if (!_pmcmc)
    paramError("sampler", "The selected sampler is not an MCMC sampler.");

  const auto rows = _sampler.getNumberOfRows();
  const auto cols = _sampler.getNumberOfCols();

  // Create communicator that only has processors with rows
  _communicator.split(
      _sampler.getNumberOfLocalRows() > 0 ? 1 : MPI_UNDEFINED, processor_id(), _local_comm);
  
  _inputs.resize(cols);
  for (unsigned int i = 0; i < cols; ++i)
    _inputs[i].resize(rows-1);

  _output_required.resize(rows)
}

void 
computeTransitionVector(std::vector<Real> tv, std::vector<const Distribution *> priors, std::vector<const Likelihood *> likelihoods, const DenseMatrix<Real> & inputs, const std::vector<Real> & outputs)
{
  Real sum1 = 0.0;
  Real quant1;
  for (unsigned int i = 0; i < tv.size()-1; ++i)
  {
    quant1 = 0.0;
    for (unsigned int j = 0; j < priors.size(); ++j)
        quant1 += (std::log(priors[j]->pdf(inputs(i, j))) - std::log(priors[j]->pdf(inputs(tv.size(), j))));
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
        quant1 += (likelihoods[j]->function(outputs[i]) - likelihoods[j]->function(outputs[tv.size()]))
    quant1 += std::log(1 / (tv.size()-1));
    tv[i] = quant1;
    sum1 += quant1;
  }
  tv[tv.size()] = sum1;
}

void
ParallelMarkovChainMonteCarloDecision::execute()
{
  
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
  
  std::vector<Real> tpm;
  tpm.resize(_sampler.getNumberOfRows());
  computeTransitionVector(tpm, _priors, _likelihoods);

  resample(data_in, _inputs, tpm, _sampler.getNumberOfRows()-1);

}
