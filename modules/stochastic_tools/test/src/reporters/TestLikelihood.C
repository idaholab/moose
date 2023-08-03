//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestLikelihood.h"
#include "Sampler.h"

registerMooseObject("StochasticToolsTestApp", TestLikelihood);

InputParameters
TestLikelihood::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += LikelihoodInterface::validParams();
  params.addClassDescription("Reporter to test a likelihood object.");
  params.addParam<ReporterValueName>(
      "function", "function", "Value of the density or mass function.");
  params.addRequiredParam<std::vector<UserObjectName>>("likelihoods", "Names of likelihoods.");
  params.addRequiredParam<ReporterName>("model_pred", "Reporter with the model predictions.");
  params.addParam<ReporterValueName>(
      "model_pred_required",
      "model_pred_required",
      "Modified value of the model output from this reporter class.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  return params;
}

TestLikelihood::TestLikelihood(const InputParameters & parameters)
  : GeneralReporter(parameters),
    LikelihoodInterface(parameters),
    _function(declareValue<std::vector<Real>>("function")),
    _model_pred(getReporterValue<std::vector<Real>>("model_pred", REPORTER_MODE_DISTRIBUTED)),
    _model_pred_required(declareValue<std::vector<Real>>("model_pred_required")),
    _sampler(getSampler("sampler")),
    _local_comm(_sampler.getLocalComm())
{
  for (const UserObjectName & name : getParam<std::vector<UserObjectName>>("likelihoods"))
    _likelihoods.push_back(getLikelihoodFunctionByName(name));

  _function.resize(_likelihoods.size());
}

void
TestLikelihood::execute()
{
  _model_pred_required = _model_pred;
  _local_comm.allgather(_model_pred_required);
  for (unsigned i = 0; i < _function.size(); ++i)
    _function[i] = _likelihoods[i]->function(_model_pred_required);
}
