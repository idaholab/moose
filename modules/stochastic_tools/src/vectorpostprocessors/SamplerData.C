//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "SamplerData.h"

// MOOSE includes
#include "Sampler.h"

registerMooseObject("StochasticToolsApp", SamplerData);

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
  // The execute method computes the complete vectors on all processes, so broadcasting the data
  // is not required.
  params.set<bool>("_is_broadcast") = true;
  return params;
}

SamplerData::SamplerData(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters), SamplerInterface(this), _sampler(getSampler("sampler"))
{
  for (dof_id_type i = 0; i < _sampler.getNumberOfCols(); ++i)
    _sample_vectors.push_back(
        &declareVector(getParam<SamplerName>("sampler") + "_" + std::to_string(i)));
}

void
SamplerData::initialize()
{
}

void
SamplerData::execute()
{
  DenseMatrix<Real> data = _sampler.getSamples();
  for (unsigned int j = 0; j < data.n(); ++j)
  {
    _sample_vectors[j]->resize(data.m());
    for (unsigned int i = 0; i < data.m(); ++i)
      (*_sample_vectors[j])[i] = data(i, j);
  }
}
