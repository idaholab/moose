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
  params.addClassDescription("Base class for covariance functions");
  params.registerBase("CovarianceFunctionBase");
  params.registerSystemAttributeName("CovarianceFunctionBase");
  return params;
}

CovarianceFunctionBase::CovarianceFunctionBase(const InputParameters & parameters)
  : MooseObject(parameters)
{
}

void
CovarianceFunctionBase::computedKdhyper(RealEigenMatrix & /*dKdhp*/,
                                        const RealEigenMatrix & /*x*/,
                                        std::string /*hyper_param_name*/,
                                        unsigned int /*ind*/) const
{
  mooseError("Hyperparameter tuning not set up for this covariance function. Please define "
             "computedKdhyper() to compute gradient.");
}

const Real &
CovarianceFunctionBase::addRealHyperParameter(const std::string & name, const Real value)
{
  return _hp_map_real.emplace(name, value).first->second;
}

const std::vector<Real> &
CovarianceFunctionBase::addVectorRealHyperParameter(const std::string & name,
                                                    const std::vector<Real> value)
{
  return _hp_map_vector_real.emplace(name, value).first->second;
}

bool
CovarianceFunctionBase::isTunable(std::string name) const
{
  if (_tunable_hp.find(name) != _tunable_hp.end())
    return true;
  else if (isParamValid(name))
    mooseError("Tuning not supported for parameter ", name);
  else
    mooseError("Parameter ", name, " selected for tuning is not a valid parameter");
  return false;
}

void
CovarianceFunctionBase::loadHyperParamMap(
    const std::unordered_map<std::string, Real> & map,
    const std::unordered_map<std::string, std::vector<Real>> & vec_map)
{
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
  for (auto iter : _hp_map_real)
    map[iter.first] = iter.second;
  for (auto iter : _hp_map_vector_real)
    vec_map[iter.first] = iter.second;
}

void
CovarianceFunctionBase::getTuningData(std::string name,
                                      unsigned int & size,
                                      Real & min,
                                      Real & max) const
{
  min = 1e-9;
  max = 1e9;

  if (_hp_map_real.find(name) != _hp_map_real.end())
    size = 1;
  else if (_hp_map_vector_real.find(name) != _hp_map_vector_real.end())
  {
    const auto & vector_value = _hp_map_vector_real.find(name);
    size = vector_value->second.size();
  }
  else
    size = 0;
}
