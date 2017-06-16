/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

// Stocastic Tools Includes
#include "SamplerData.h"

// MOOSE includes
#include "Sampler.h"

template <>
InputParameters
validParams<SamplerData>()
{
  InputParameters params = validParams<GeneralVectorPostprocessor>();
  params += validParams<SamplerInterface>();
  params.addRequiredParam<SamplerName>("sampler",
                                       "The sample from which to extract distribution data.");
  return params;
}

SamplerData::SamplerData(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), SamplerInterface(this), _sampler(getSampler("sampler"))
{
  for (auto & name : _sampler.getDistributionNames())
    _sample_vectors.push_back(&declareVector(name));
}

void
SamplerData::initialize()
{
  for (VectorPostprocessorValue * ptr : _sample_vectors)
    ptr->clear();
}

void
SamplerData::execute()
{
  std::vector<DenseMatrix<Real>> data = _sampler.getSamples();
  for (std::size_t i = 0; i < _sample_vectors.size(); ++i)
    _sample_vectors[i]->assign(data[i].get_values().begin(), data[i].get_values().end());
}
