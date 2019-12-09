//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "StochasticResults.h"

// MOOSE includes
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", StochasticResults);

template <>
InputParameters
validParams<StochasticResults>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params.addClassDescription(
      "Storage container for stochastic simulation results coming from a Postprocessor.");
  params += validParams<SamplerInterface>();
  return params;
}

StochasticResults::StochasticResults(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), SamplerInterface(this)
{
}

void
StochasticResults::initialize()
{
  _sample_vector = &declareVector(_sampler->name());
  _sample_vector->resize(_sampler->getNumberOfRows(), 0);
}

void
StochasticResults::init(Sampler & sampler)
{
  _sampler = &sampler;
  _sample_vector = &declareVector(sampler.name());
}
