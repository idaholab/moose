//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "PODSurrogateTester.h"

#include "Sampler.h"

registerMooseObject("StochasticToolsTestApp", PODSurrogateTester);

InputParameters
PODSurrogateTester::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription("Tool for sampling POD surrogate model.");
  params.addRequiredParam<std::vector<UserObjectName>>("model", "Name of POD surrogate models.");
  params += SamplerInterface::validParams();
  params.addRequiredParam<SamplerName>("sampler",
                                       "Sampler to use for evaluating surrogate models.");
  params.addParam<bool>(
      "output_samples",
      false,
      "True to output value of parameter values from samples (this may be VERY large).");
  params.addRequiredParam<std::string>(
      "variable_name", "The name of the variable this prostprocessor is supposed to operate on.");
  MultiMooseEnum pptype("nodal_max=0 nodal_min=1 nodal_l1=2 nodal_l2=3 nodal_linf=4");
  params.addRequiredParam<MultiMooseEnum>(
      "to_compute", pptype, "The global data the postprocessor should compute.");
  return params;
}

PODSurrogateTester::PODSurrogateTester(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SurrogateModelInterface(this),
    _sampler(getSampler("sampler")),
    _output_samples(getParam<bool>("output_samples")),
    _variable_name(getParam<std::string>("variable_name")),
    _to_compute(getParam<MultiMooseEnum>("to_compute"))
{
  const auto & model_names = getParam<std::vector<UserObjectName>>("model");
  _model.reserve(model_names.size());
  _value_vector.reserve(model_names.size());

  for (unsigned int model_i = 0; model_i < model_names.size(); ++model_i)
  {
    // Adding surrogate models first
    _model.push_back(&getSurrogateModelByName<PODReducedBasisSurrogate>(model_names[model_i]));

    // Creating given vector postprocessors for every item in to_compute
    for (unsigned int pp_i = 0; pp_i < _to_compute.size(); ++pp_i)
    {
      std::string name = model_names[model_i] + ":" + _to_compute[pp_i];
      _value_vector.push_back(&declareVector(name));
    }
  }

  if (_output_samples)
    for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      _sample_vector.push_back(&declareVector("sample_p" + std::to_string(d)));
}

void
PODSurrogateTester::initialize()
{
  for (auto & vec : _value_vector)
    vec->resize(_sampler.getNumberOfLocalRows(), 0);

  if (_output_samples)
    for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      _sample_vector[d]->resize(_sampler.getNumberOfLocalRows(), 0);
}

void
PODSurrogateTester::execute()
{
  unsigned int n_models = _model.size();
  unsigned int n_pp = _to_compute.size();

  // Loop over samples
  for (dof_id_type p = _sampler.getLocalRowBegin(); p < _sampler.getLocalRowEnd(); ++p)
  {
    std::vector<Real> data = _sampler.getNextLocalRow();

    for (unsigned int m = 0; m < n_models; ++m)
    {
      _model[m]->evaluateSolution(data);
      for (unsigned int ppi = 0; ppi < n_pp; ++ppi)
      {
        unsigned int idx = m * n_pp + ppi;
        (*_value_vector[idx])[p - _sampler.getLocalRowBegin()] =
            _model[m]->getNodalQoI(_variable_name, _to_compute.get(ppi));
      }
    }

    if (_output_samples)
      for (unsigned int d = 0; d < _sampler.getNumberOfCols(); ++d)
      {
        (*_sample_vector[d])[p - _sampler.getLocalRowBegin()] = data[d];
      }
  }
}

void
PODSurrogateTester::finalize()
{
  for (auto & vec : _value_vector)
    _communicator.gather(0, *vec);
  if (_output_samples)
    for (auto & ppv_ptr : _sample_vector)
      _communicator.gather(0, *ppv_ptr);
}
