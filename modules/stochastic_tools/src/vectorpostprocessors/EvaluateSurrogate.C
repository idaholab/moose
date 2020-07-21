//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "EvaluateSurrogate.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsApp", EvaluateSurrogate);

InputParameters
EvaluateSurrogate::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SamplerInterface::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription("Tool for sampling surrogate models.");
  params.addRequiredParam<std::vector<UserObjectName>>("model", "Name of surrogate models.");
  params += SamplerInterface::validParams();
  params.addRequiredParam<SamplerName>("sampler",
                                       "Sampler to use for evaluating surrogate models.");
  params.addParam<bool>(
      "output_samples",
      false,
      "True to output value of parameter values from samples (this may be VERY large).");
  return params;
}

EvaluateSurrogate::EvaluateSurrogate(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerInterface(this),
    SurrogateModelInterface(this),
    _sampler(getSampler("sampler")),
    _output_samples(getParam<bool>("output_samples"))
{
  const auto & model_names = getParam<std::vector<UserObjectName>>("model");
  _model.reserve(model_names.size());
  _value_vector.reserve(model_names.size());
  for (const auto & nm : model_names)
  {
    _model.push_back(&getSurrogateModelByName(nm));
    _value_vector.push_back(&declareVector(nm));
  }

  if (_output_samples)
    for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      _sample_vector.push_back(&declareVector("sample_p" + std::to_string(d)));
}

void
EvaluateSurrogate::initialize()
{
  for (auto & vec : _value_vector)
    vec->resize(_sampler.getNumberOfLocalRows(), 0);

  if (_output_samples)
    for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      _sample_vector[d]->resize(_sampler.getNumberOfLocalRows(), 0);
}

void
EvaluateSurrogate::execute()
{
  // Loop over samples
  for (dof_id_type p = _sampler.getLocalRowBegin(); p < _sampler.getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler.getNextLocalRow();
    for (unsigned int m = 0; m < _model.size(); ++m)
      (*_value_vector[m])[p - _sampler.getLocalRowBegin()] = _model[m]->evaluate(data);
    if (_output_samples)
      for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      {
        (*_sample_vector[d])[p - _sampler.getLocalRowBegin()] = data[d];
      }
  }
}

void
EvaluateSurrogate::finalize()
{
  for (auto & vec : _value_vector)
    _communicator.gather(0, *vec);
  if (_output_samples)
    for (auto & ppv_ptr : _sample_vector)
      _communicator.gather(0, *ppv_ptr);
}
