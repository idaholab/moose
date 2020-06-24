//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "GaussianProcessTester.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsTestApp", GaussianProcessTester);

InputParameters
GaussianProcessTester::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SamplerInterface::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription("Tool for sampling Gaussian Process surrogate models.");
  params.addRequiredParam<UserObjectName>("model", "Name of surrogate model.");
  params += SamplerInterface::validParams();
  params.addRequiredParam<SamplerName>(
      "sampler", "Sampler to use for evaluating PCE model (mainly for testing).");
  params.addParam<bool>("output_samples",
                        false,
                        "True to output value of samples from sampler (this may be VERY large).");
  return params;
}

GaussianProcessTester::GaussianProcessTester(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerInterface(this),
    SurrogateModelInterface(this),
    _sampler(getSampler("sampler")),
    _output_samples(getParam<bool>("output_samples")),
    _model(getSurrogateModel<GaussianProcess>("model")),
    _mean_vector(declareVector("mean")),
    _std_vector(declareVector("std"))
{
  if (_output_samples)
    for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      _sample_vector.push_back(&declareVector("sample_p" + std::to_string(d)));
}

void
GaussianProcessTester::initialize()
{
  _mean_vector.resize(_sampler.getNumberOfLocalRows(), 0);
  _std_vector.resize(_sampler.getNumberOfLocalRows(), 0);
  if (_output_samples)
    for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      _sample_vector[d]->resize(_sampler.getNumberOfLocalRows(), 0);
}

void
GaussianProcessTester::execute()
{
  // Loop over samples
  for (dof_id_type p = _sampler.getLocalRowBegin(); p < _sampler.getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler.getNextLocalRow();
    Real std;
    _mean_vector[p - _sampler.getLocalRowBegin()] = _model.evaluate(data, std);
    _std_vector[p - _sampler.getLocalRowBegin()] = std;
    if (_output_samples)
      for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      {
        (*_sample_vector[d])[p - _sampler.getLocalRowBegin()] = data[d];
      }
  }
}

void
GaussianProcessTester::finalize()
{
  _communicator.gather(0, _mean_vector);
  _communicator.gather(0, _std_vector);
  if (_output_samples)
    for (auto & ppv_ptr : _sample_vector)
      _communicator.gather(0, *ppv_ptr);
}
