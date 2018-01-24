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
  mooseAssert(_sampler, "The _sampler pointer must be initialized via the init() method.");

  // Resize and zero vectors to the correct size, this allows the SamplerPostprocessorTransfer
  // to set values in the vector directly.
  std::vector<DenseMatrix<Real>> data = _sampler->getSamples();
  for (auto i = beginIndex(data); i < data.size(); ++i)
    _sample_vectors[i]->resize(data[i].m(), 0);
}

VectorPostprocessorValue &
StochasticResults::getVectorPostprocessorValueByGroup(unsigned int group)
{
  if (group >= _sample_vectors.size())
    mooseError("The supplied sample index ", group, " does not exist.");
  return *_sample_vectors[group];
}

void
StochasticResults::init(Sampler & sampler)
{
  _sampler = &sampler;
  const std::vector<std::string> & names = _sampler->getSampleNames();
  _sample_vectors.resize(names.size());
  for (auto i = beginIndex(names); i < names.size(); ++i)
    _sample_vectors[i] = &declareVector(names[i]);
}
