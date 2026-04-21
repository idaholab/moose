//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#ifdef MOOSE_LIBTORCH_ENABLED

#include "CovarianceFunctionBase.h"
#include "LibtorchUtils.h"

namespace
{

torch::Tensor
makeScalarHyperParameter(const Real value)
{
  return torch::tensor(value, torch::TensorOptions().dtype(at::kDouble));
}

torch::Tensor
makeVectorHyperParameter(const std::vector<Real> & value)
{
  return LibtorchUtils::vectorToTensorView(value, {long(value.size())}).clone();
}

torch::Tensor &
insertHyperParameter(std::unordered_map<std::string, torch::Tensor> & hyperparameters,
                     std::unordered_set<std::string> & tunable_hp,
                     const std::string & prefixed_name,
                     torch::Tensor tensor,
                     const bool is_tunable)
{
  if (is_tunable)
    tunable_hp.insert(prefixed_name);
  return hyperparameters.emplace(prefixed_name, std::move(tensor)).first->second;
}

bool
isScalarHyperParameter(const torch::Tensor & tensor)
{
  return tensor.dim() == 0;
}

bool
isVectorHyperParameter(const torch::Tensor & tensor)
{
  return tensor.dim() == 1;
}

} // namespace

InputParameters
CovarianceFunctionBase::validParams()
{
  InputParameters params = MooseObject::validParams();
  params.addParam<std::vector<UserObjectName>>(
      "covariance_functions", {}, "Covariance functions that this covariance function depends on.");
  params.addParam<unsigned int>(
      "num_outputs", 1, "The number of outputs expected for this covariance function.");
  params.addClassDescription("Base class for covariance functions");
  params.registerBase("CovarianceFunctionBase");
  params.registerSystemAttributeName("CovarianceFunction");
  return params;
}

CovarianceFunctionBase::CovarianceFunctionBase(const InputParameters & parameters)
  : MooseObject(parameters),
    CovarianceInterface(parameters),
    _num_outputs(getParam<unsigned int>("num_outputs")),
    _dependent_covariance_names(getParam<std::vector<UserObjectName>>("covariance_functions"))

{
  // Fetch the dependent covariance functions
  for (const auto & name : _dependent_covariance_names)
  {
    _covariance_functions.push_back(getCovarianceFunctionByName(name));
    _dependent_covariance_types.push_back(_covariance_functions.back()->type());
  }
}

bool
CovarianceFunctionBase::computedKdhyper(torch::Tensor & /*dKdhp*/,
                                        const torch::Tensor & /*x*/,
                                        const std::string & /*hyper_param_name*/,
                                        unsigned int /*ind*/) const
{
  mooseError("Hyperparameter tuning not set up for this covariance function. Please define "
             "computedKdhyper() to compute gradient.");
}

torch::Tensor &
CovarianceFunctionBase::addRealHyperParameter(const std::string & name,
                                              const Real value,
                                              const bool is_tunable)
{
  const auto prefixed_name = _name + ":" + name;
  return insertHyperParameter(
      _hyperparameters, _tunable_hp, prefixed_name, makeScalarHyperParameter(value), is_tunable);
}

torch::Tensor &
CovarianceFunctionBase::addVectorRealHyperParameter(const std::string & name,
                                                    const std::vector<Real> & value,
                                                    const bool is_tunable)
{
  const auto prefixed_name = _name + ":" + name;
  return insertHyperParameter(
      _hyperparameters, _tunable_hp, prefixed_name, makeVectorHyperParameter(value), is_tunable);
}

bool
CovarianceFunctionBase::isTunable(const std::string & name) const
{
  // First, we check if the dependent covariances have the parameter
  for (const auto dependent_covar : _covariance_functions)
    if (dependent_covar->isTunable(name))
      return true;

  if (_tunable_hp.find(name) != _tunable_hp.end())
    return true;
  else if (_hyperparameters.find(name) != _hyperparameters.end())
    mooseError("We found hyperparameter ", name, " but it was not declared tunable!");

  return false;
}

void
CovarianceFunctionBase::loadHyperParamMap(const HyperParameterMap & map)
{
  // First, load the hyperparameters of the dependent covariance functions
  for (const auto dependent_covar : _covariance_functions)
    dependent_covar->loadHyperParamMap(map);

  // Then we load the hyperparameters of this object
  for (auto & iter : _hyperparameters)
  {
    const auto map_iter = map.find(iter.first);
    if (map_iter == map.end())
      continue;

    if (!isScalarHyperParameter(map_iter->second) && !isVectorHyperParameter(map_iter->second))
      mooseError("Unsupported hyperparameter rank ", map_iter->second.dim(), " for ", iter.first, ".");

    iter.second = map_iter->second.clone();
  }
}

void
CovarianceFunctionBase::buildHyperParamMap(HyperParameterMap & map) const
{
  // First, add the hyperparameters of the dependent covariance functions
  for (const auto dependent_covar : _covariance_functions)
    dependent_covar->buildHyperParamMap(map);

  // At the end we just append the hyperparameters this object owns
  for (const auto & iter : _hyperparameters)
    if (!isScalarHyperParameter(iter.second) && !isVectorHyperParameter(iter.second))
      mooseError("Unsupported hyperparameter rank ", iter.second.dim(), " for ", iter.first, ".");
    else
      map[iter.first] = iter.second.clone();
}

bool
CovarianceFunctionBase::getTuningData(const std::string & name,
                                      unsigned int & size,
                                      Real & min,
                                      Real & max) const
{
  // First, check the dependent covariances
  for (const auto dependent_covar : _covariance_functions)
    if (dependent_covar->getTuningData(name, size, min, max))
      return true;

  min = 1e-9;
  max = 1e9;

  const auto tensor_value = _hyperparameters.find(name);
  if (tensor_value == _hyperparameters.end())
  {
    size = 0;
    return false;
  }

  if (isScalarHyperParameter(tensor_value->second))
  {
    size = 1;
    return true;
  }

  if (isVectorHyperParameter(tensor_value->second))
  {
    size = tensor_value->second.numel();
    return true;
  }

  mooseError("Unsupported hyperparameter rank ", tensor_value->second.dim(), " for ", name, ".");
}

void
CovarianceFunctionBase::dependentCovarianceTypes(
    std::map<UserObjectName, std::string> & name_type_map) const
{
  for (const auto dependent_covar : _covariance_functions)
  {
    dependent_covar->dependentCovarianceTypes(name_type_map);
    name_type_map.insert(std::make_pair(dependent_covar->name(), dependent_covar->type()));
  }
}

#endif
