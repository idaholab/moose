//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "TestLikelihood.h"

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
  return params;
}

TestLikelihood::TestLikelihood(const InputParameters & parameters)
  : GeneralReporter(parameters),
    LikelihoodInterface(parameters),
    _function(declareValue<std::vector<Real>>("function")),
    _model_pred(getReporterValue<std::vector<Real>>("model_pred"))
{
  for (const UserObjectName & name : getParam<std::vector<UserObjectName>>("likelihoods"))
    _likelihoods.push_back(getLikelihoodFunctionByName(name));

  _function.resize(_likelihoods.size());
}

void
TestLikelihood::execute()
{
  for (unsigned i = 0; i < _function.size(); ++i)
    _function[i] = _likelihoods[i]->function(_model_pred);
}
