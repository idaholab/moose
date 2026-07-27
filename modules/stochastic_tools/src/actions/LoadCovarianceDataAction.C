//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_LIBTORCH_ENABLED

#include "LoadCovarianceDataAction.h"
#include "CovarianceFunctionBase.h"
#include "GaussianProcessSurrogate.h"
#include "FEProblem.h"
#include "StochasticToolsApp.h"

registerMooseAction("StochasticToolsApp", LoadCovarianceDataAction, "load_covariance_data");

namespace
{

using HyperParameterMap = CovarianceFunctionBase::HyperParameterMap;

std::vector<Real>
exportHyperParameter(const torch::Tensor & tensor)
{
  const auto flattened = tensor.reshape({-1}).contiguous();
  return {flattened.data_ptr<Real>(), flattened.data_ptr<Real>() + flattened.numel()};
}

void
assignRequiredHyperParameter(InputParameters & params,
                             const std::string & param_name,
                             const torch::Tensor & tensor)
{
  if (params.have_parameter<Real>(param_name))
  {
    if (!CovarianceFunctionBase::isScalarHyperParameter(tensor))
      mooseError("Expected scalar hyperparameter for ", param_name, ".");
    params.set<Real>(param_name) = tensor.item<Real>();
  }
  else if (params.have_parameter<unsigned int>(param_name))
  {
    if (!CovarianceFunctionBase::isScalarHyperParameter(tensor))
      mooseError("Expected scalar hyperparameter for ", param_name, ".");
    params.set<unsigned int>(param_name) = cast_int<unsigned int>(tensor.item<Real>());
  }
  else if (params.have_parameter<std::vector<Real>>(param_name))
  {
    if (!CovarianceFunctionBase::isVectorHyperParameter(tensor))
      mooseError("Expected vector hyperparameter for ", param_name, ".");
    params.set<std::vector<Real>>(param_name) = exportHyperParameter(tensor);
  }
  else
    mooseError("Unsupported hyperparameter type for ", param_name, ".");
}

void
loadRequiredHyperParameter(InputParameters & params,
                           const UserObjectName & object_name,
                           const std::string & param_name,
                           const HyperParameterMap & hyperparameters)
{
  const auto expected_name = std::string(object_name) + ":" + param_name;
  const auto hyperparam_it = hyperparameters.find(expected_name);
  if (hyperparam_it != hyperparameters.end())
    assignRequiredHyperParameter(params, param_name, hyperparam_it->second);
}

} // namespace

InputParameters
LoadCovarianceDataAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Calls load method on SurrogateModel objects contained within the "
                             "`[Surrogates]` input block, if a filename is given.");
  return params;
}

LoadCovarianceDataAction::LoadCovarianceDataAction(const InputParameters & params) : Action(params)
{
}

void
LoadCovarianceDataAction::act()
{
  std::vector<SurrogateModel *> objects;
  _app.theWarehouse().query().condition<AttribSystem>("SurrogateModel").queryInto(objects);
  for (auto model_ptr : objects)
  {
    auto * gp_gen = dynamic_cast<GaussianProcessSurrogate *>(model_ptr);
    if (gp_gen && model_ptr->isParamValid("filename"))
      load(*gp_gen);
  }
}

void
LoadCovarianceDataAction::load(GaussianProcessSurrogate & model)
{
  // We grab all the necessary information that is needed to reconstruct the
  // covariance structure for the GP
  const std::string & covar_type = model.getGP().getCovarType();
  const std::string & covar_name = model.getGP().getCovarName();
  const std::map<UserObjectName, std::string> & dep_covar_types =
      model.getGP().getDependentCovarTypes();
  const std::vector<UserObjectName> & dep_covar_names = model.getGP().getDependentCovarNames();

  // This is for the covariance on the very top, the lower-level covariances are
  // all assumed to have num_outputs=1.
  const unsigned int num_outputs = model.getGP().getCovarNumOutputs();
  const HyperParameterMap & hyperparameters = model.getGP().getHyperParamMap();

  // We start by creating and loading the lower-level covariances if they need
  // to be present. Right now we can only load a complex covariance which has
  // a one-level dependency depth.
  // TODO: Extend this to arbitrary dependency depths. Maybe we could use a graph.
  for (const auto & it : dep_covar_types)
  {
    const auto & name = it.first;
    const auto & type = it.second;
    InputParameters covar_params = _factory.getValidParams(type);

    // We make sure that every required parameter is added so that the object
    // can be constructed. The non-required hyperparameters (if present in the
    // parameter maps) will be inserted later.
    const auto param_list = covar_params.getParametersList();
    for (const auto & param : param_list)
      if (covar_params.isParamRequired(param))
        loadRequiredHyperParameter(covar_params, name, param, hyperparameters);

    _problem->addObject<CovarianceFunctionBase>(type, name, covar_params, /*threaded=*/false);
  }

  InputParameters covar_params = _factory.getValidParams(covar_type);
  covar_params.set<unsigned int>("num_outputs") = num_outputs;
  covar_params.set<std::vector<UserObjectName>>("covariance_functions") = dep_covar_names;

  const auto param_list = covar_params.getParametersList();
  for (const auto & param : param_list)
    // We make sure that every required parameter is added so that the object
    // can be constructed. The non-required hyperparameters (if present in the
    // parameter maps) will be inserted later.
    if (covar_params.isParamRequired(param))
      loadRequiredHyperParameter(covar_params, covar_name, param, hyperparameters);

  auto covar_object = _problem->addObject<CovarianceFunctionBase>(
      covar_type, covar_name, covar_params, /* threaded = */ false);
  covar_object[0]->loadHyperParamMap(hyperparameters);

  model.setupCovariance(covar_name);
}

#endif
