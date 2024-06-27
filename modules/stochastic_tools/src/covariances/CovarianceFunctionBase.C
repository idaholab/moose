//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CovarianceFunctionBase.h"

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
CovarianceFunctionBase::computedKdhyper(RealEigenMatrix & /*dKdhp*/,
                                        const RealEigenMatrix & /*x*/,
                                        const std::string & /*hyper_param_name*/,
                                        unsigned int /*ind*/) const
{
  mooseError("Hyperparameter tuning not set up for this covariance function. Please define "
             "computedKdhyper() to compute gradient.");
}

const Real &
CovarianceFunctionBase::addRealHyperParameter(const std::string & name,
                                              const Real value,
                                              const bool is_tunable)
{
  const auto prefixed_name = _name + ":" + name;
  if (is_tunable)
    _tunable_hp.insert(prefixed_name);
  return _hp_map_real.emplace(prefixed_name, value).first->second;
}

const std::vector<Real> &
CovarianceFunctionBase::addVectorRealHyperParameter(const std::string & name,
                                                    const std::vector<Real> value,
                                                    const bool is_tunable)
{
  const auto prefixed_name = _name + ":" + name;
  if (is_tunable)
    _tunable_hp.insert(prefixed_name);
  return _hp_map_vector_real.emplace(prefixed_name, value).first->second;
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
  else if (_hp_map_real.find(name) != _hp_map_real.end() ||
           _hp_map_vector_real.find(name) != _hp_map_vector_real.end())
    mooseError("We found hyperparameter ", name, " but it was not declared tunable!");

  return false;
}

void
CovarianceFunctionBase::loadHyperParamMap(
    const std::unordered_map<std::string, Real> & map,
    const std::unordered_map<std::string, std::vector<Real>> & vec_map)
{
  // First, load the hyperparameters of the dependent covariance functions
  for (const auto dependent_covar : _covariance_functions)
    dependent_covar->loadHyperParamMap(map, vec_map);

  // Then we load the hyperparameters of this object
  for (auto & iter : _hp_map_real)
  {
    const auto & map_iter = map.find(iter.first);
    if (map_iter != map.end())
      iter.second = map_iter->second;
  }
  for (auto & iter : _hp_map_vector_real)
  {
    const auto & map_iter = vec_map.find(iter.first);
    if (map_iter != vec_map.end())
      iter.second = map_iter->second;
  }
}

void
CovarianceFunctionBase::buildHyperParamMap(
    std::unordered_map<std::string, Real> & map,
    std::unordered_map<std::string, std::vector<Real>> & vec_map) const
{
  // First, add the hyperparameters of the dependent covariance functions
  for (const auto dependent_covar : _covariance_functions)
    dependent_covar->buildHyperParamMap(map, vec_map);

  // At the end we just append the hyperparameters this object owns
  for (const auto & iter : _hp_map_real)
    map[iter.first] = iter.second;
  for (const auto & iter : _hp_map_vector_real)
    vec_map[iter.first] = iter.second;
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

  if (_hp_map_real.find(name) != _hp_map_real.end())
  {
    size = 1;
    return true;
  }
  else if (_hp_map_vector_real.find(name) != _hp_map_vector_real.end())
  {
    const auto & vector_value = _hp_map_vector_real.find(name);
    size = vector_value->second.size();
    return true;
  }
  else
  {
    size = 0;
    return false;
  }
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
