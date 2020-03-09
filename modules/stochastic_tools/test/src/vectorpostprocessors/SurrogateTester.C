//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "SurrogateTester.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsTestApp", SurrogateTester);

defineLegacyParams(SurrogateTester);

InputParameters
SurrogateTester::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params.addClassDescription("Tool for sampling surrogate model.");
  params.addRequiredParam<UserObjectName>("model", "Name of surrogate model.");
  params += SamplerInterface::validParams();
  params.addRequiredParam<SamplerName>(
      "sampler", "Sampler to use for evaluating PCE model (mainly for testing).");
  params.addParam<bool>("output_samples",
                        false,
                        "True to output value of samples from sampler (this may be VERY large).");
  return params;
}

SurrogateTester::SurrogateTester(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SamplerInterface(this),
    _sampler(getSampler("sampler")),
    _output_samples(getParam<bool>("output_samples")),
    _model(getUserObject<SurrogateModel>("model")),
    _value_vector(declareVector("value"))
{
  if (_output_samples)
    for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      _sample_vector.push_back(&declareVector("sample_p" + std::to_string(d)));
}

void
SurrogateTester::initialize()
{
  mooseAssert(_sampler.getNumberOfCols() == _model.getNumberOfParameters(),
              "Number of sampler columns does not match number of parameters in PCE.");

  _value_vector.resize(_sampler.getNumberOfLocalRows(), 0);
  if (_output_samples)
    for (unsigned int d = 0; d < _model.getNumberOfParameters(); ++d)
      _sample_vector[d]->resize(_sampler.getNumberOfLocalRows(), 0);
}

void
SurrogateTester::execute()
{
  // Loop over samples
  for (dof_id_type p = _sampler.getLocalRowBegin(); p < _sampler.getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler.getNextLocalRow();
    _value_vector[p - _sampler.getLocalRowBegin()] = _model.evaluate(data);
    if (_output_samples)
      for (unsigned int d = 0; d < _model.getNumberOfParameters(); ++d)
      {
        (*_sample_vector[d])[p - _sampler.getLocalRowBegin()] = data[d];
      }
  }
}

void
SurrogateTester::finalize()
{
  _communicator.gather(0, _value_vector);
  if (_output_samples)
    for (auto & ppv_ptr : _sample_vector)
      _communicator.gather(0, *ppv_ptr);
}
