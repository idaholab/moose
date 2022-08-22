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
  InputParameters params = StochasticReporter::validParams();
  params += SurrogateModelInterface::validParams();
  params += SamplerInterface::validParams();
  params.addClassDescription("Tool for sampling surrogate models.");
  params.addRequiredParam<std::vector<UserObjectName>>("model", "Name of surrogate models.");
  params.addRequiredParam<SamplerName>("sampler",
                                       "Sampler to use for evaluating surrogate models.");
  MultiMooseEnum rtypes(SurrogateModel::defaultResponseTypes().getRawNames(), "real");
  params.addParam<MultiMooseEnum>(
      "response_type",
      rtypes,
      "The type of return value expected from the surrogate models, a single entry will use it for "
      "every model. Warning: not every model is able evaluate every response type.");
  MultiMooseEnum estd("false=0 true=1", "false");
  params.addParam<MultiMooseEnum>(
      "evaluate_std",
      estd,
      "Whether or not to evaluate standard deviation associated with each sample, a single entry "
      "will use it for every model. Warning: not every model can compute standard deviation.");
  return params;
}

EvaluateSurrogate::EvaluateSurrogate(const InputParameters & parameters)
  : StochasticReporter(parameters),
    SurrogateModelInterface(this),
    _sampler(getSampler("sampler")),
    _response_types(getParam<MultiMooseEnum>("response_type"))
{
  const auto & model_names = getParam<std::vector<UserObjectName>>("model");
  _model.reserve(model_names.size());
  for (const auto & nm : model_names)
    _model.push_back(&getSurrogateModelByName(nm));

  if (_response_types.size() != 1 && _response_types.size() != _model.size())
    paramError("response_type",
               "Number of entries must be 1 or equal to the number of entries in 'model'.");

  const auto & estd = getParam<MultiMooseEnum>("evaluate_std");
  if (estd.size() != 1 && estd.size() != _model.size())
    paramError("evaluate_std",
               "Nmber of entries must be 1 or equal to the number of entries in 'model'.");
  _doing_std.resize(_model.size());
  for (const auto i : index_range(_model))
    _doing_std[i] = estd.size() == 1 ? estd[0] == "true" : estd[i] == "true";

  _real_values.resize(_model.size(), nullptr);
  _real_std.resize(_model.size(), nullptr);
  _vector_real_values.resize(_model.size(), nullptr);
  _vector_real_std.resize(_model.size(), nullptr);
  for (const auto i : index_range(_model))
  {
    const std::string rtype = _response_types.size() == 1 ? _response_types[0] : _response_types[i];
    if (rtype == "real")
    {
      _real_values[i] = &declareStochasticReporter<Real>(model_names[i], _sampler);
      if (_doing_std[i])
        _real_std[i] = &declareStochasticReporter<Real>(model_names[i] + "_std", _sampler);
    }
    else if (rtype == "vector_real")
    {
      _vector_real_values[i] =
          &declareStochasticReporter<std::vector<Real>>(model_names[i], _sampler);
      if (_doing_std[i])
        _vector_real_std[i] =
            &declareStochasticReporter<std::vector<Real>>(model_names[i] + "_std", _sampler);
    }
    else
      paramError("response_type", "Unknown response type ", _response_types[i]);
  }
}

void
EvaluateSurrogate::execute()
{
  // Loop over samples
  for (const auto ind : make_range(_sampler.getNumberOfLocalRows()))
  {
    const std::vector<Real> data = _sampler.getNextLocalRow();
    for (const auto m : index_range(_model))
    {
      if (_real_values[m] && _real_std[m])
        (*_real_values[m])[ind] = _model[m]->evaluate(data, (*_real_std[m])[ind]);
      else if (_real_values[m])
        (*_real_values[m])[ind] = _model[m]->evaluate(data);
      else if (_vector_real_values[m] && _vector_real_std[m])
        _model[m]->evaluate(data, (*_vector_real_values[m])[ind], (*_vector_real_std[m])[ind]);
      else if (_vector_real_values[m])
        _model[m]->evaluate(data, (*_vector_real_values[m])[ind]);
    }
  }
}
