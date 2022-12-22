//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestLikelihood.h"
#include "Likelihood.h"
#include "MooseUtils.h"

registerMooseObject("StochasticToolsApp", TestLikelihood);

InputParameters
TestLikelihood::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += LikelihoodInterface::validParams();
  params.addClassDescription("Reporter to test a likelihood object.");
  params.addParam<ReporterValueName>(
      "function", "function", "Value of the density or mass function.");
  params.addRequiredParam<std::vector<LikelihoodName>>("likelihoods", "Names of the likelihoods.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "model_predictions", "Reporters with the predictions from the models.");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "weights", "weights>0 & weights<=1", "The weights given to the different models.");
  return params;
}

TestLikelihood::TestLikelihood(const InputParameters & parameters)
  : GeneralReporter(parameters),
    LikelihoodInterface(this),
    _function(declareValue<std::vector<Real>>("function")),
    _model_pred(getParam<std::vector<ReporterName>>("model_predictions").size()),
    _weights(getParam<std::vector<Real>>("weights"))
{
  for (const LikelihoodName & name : getParam<std::vector<LikelihoodName>>("likelihoods"))
    _likelihoods.push_back(&getLikelihoodByName(name));

  const auto & rnames = getParam<std::vector<ReporterName>>("model_predictions");
  for (unsigned int i = 0; i < rnames.size(); ++i)
    _model_pred[i] = &getReporterValueByName<std::vector<Real>>(rnames[i]);

  _function.resize(_likelihoods.size());

  if (std::accumulate(_weights.begin(), _weights.end(), 0.0) != 1.0)
    mooseError("The sum of all the model weights should be equal to one.");
}

void
TestLikelihood::execute()
{
  std::vector<std::vector<Real>> model_values;
  model_values.resize(_model_pred.size());
  for (unsigned i = 0; i < _model_pred.size(); ++i)
    model_values[i] = *_model_pred[i];

  for (unsigned i = 0; i < _function.size(); ++i)
    _function[i] = _likelihoods[i]->function(model_values, _weights);
}
