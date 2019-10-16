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
  params.set<bool>("_is_broadcast") = false;

  // Flag for using getSamples or getLocalSamples, by default getLocalSamples is used. This option
  // is mainly for testing to make sure the methods return the same values.
  params.addParam<bool>("use_local_samples", true, "Toggle the use of getLocalSamples (true) or getSamples (false).");

  return params;
}

SamplerData::SamplerData(const InputParameters & parameters)
    : GeneralVectorPostprocessor(parameters), SamplerInterface(this), _sampler(getSampler("sampler")), _use_local_samples(getParam<bool>("use_local_samples"))
{
  for (dof_id_type i = 0; i < _sampler.getNumberOfCols(); ++i)
    _sample_vectors.push_back(
        &declareVector(getParam<SamplerName>("sampler") + "_" + std::to_string(i)));
}

void
SamplerData::initialize()
{
  dof_id_type n = _use_local_samples ? _sampler.getNumberOfLocalRows() : _sampler.getNumberOfRows();
  for (auto & ppv_ptr : _sample_vectors)
    ppv_ptr->resize(n, 0);
}

void
SamplerData::execute()
{
  DenseMatrix<Real> data = _use_local_samples ? _sampler.getLocalSamples() : _sampler.getSamples();
  for (unsigned int j = 0; j < data.n(); ++j)
  {
    for (unsigned int i = 0; i < data.m(); ++i)
      (*_sample_vectors[j])[i] = data(i, j);
  }
}

void
SamplerData::finalize()
{
  if (_use_local_samples)
    for (auto & ppv_ptr : _sample_vectors)
      _communicator.gather(0, *ppv_ptr);
}

void
SamplerData::threadJoin(const UserObject & /*uo*/)
{
  /// TODO: Use this when the Sampler objects become threaded GeneralUserObjects
  /*
  if (_use_local_samples)
  {
    const SamplerData & obj = static_cast<const SamplerData &>(uo);
    for (std::size_t i = 0; i < _sample_vectors.size(); ++i)
      (*_sample_vectors[i]).insert(_sample_vectors[i]->end(), obj._sample_vectors[i]->begin(),
                                   obj._sample_vectors[i]->end());
  }
  */
}
