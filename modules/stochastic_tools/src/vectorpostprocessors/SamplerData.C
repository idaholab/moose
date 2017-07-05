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
  params.addClassDescription(
      "Tool for extracting Sampler object data and storing in VectorPostprocessor vectors.");
  params += validParams<SamplerInterface>();
  params.addRequiredParam<SamplerName>("sampler",
                                       "The sample from which to extract distribution data.");
  return params;
}

SamplerData::SamplerData(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), SamplerInterface(this), _sampler(getSampler("sampler"))
{
}

void
SamplerData::initialize()
{
  for (auto ptr : _sample_vectors)
    ptr->clear();
}

void
SamplerData::execute()
{
  std::vector<DenseMatrix<Real>> data = _sampler.getSamples();
  auto n = data.size();
  if (_sample_vectors.empty())
  {
    _sample_vectors.resize(n);
    for (auto i = beginIndex(data); i < n; ++i)
    {
      std::string name = "mat_" + std::to_string(i);
      _sample_vectors[i] = &declareVector(name);
    }
  }

  for (auto i = beginIndex(data); i < n; ++i)
    _sample_vectors[i]->assign(data[i].get_values().begin(), data[i].get_values().end());
}
