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
  params.addRequiredParam<std::vector<Real>>("length_factor",
                                             "Length Factor to use for Covariance Kernel");
  params.addRequiredParam<Real>("signal_variance",
                                "Signal Variance ($\\sigma_f^2$) to use for kernel calculation.");
  params.addParam<Real>(
      "noise_variance", 0, "Noise Variance ($\\sigma_n^2$) to use for kernel calculation.");
  return params;
}

CovarianceFunctionBase::CovarianceFunctionBase(const InputParameters & parameters)
  : MooseObject(parameters),
    _length_factor(getParam<std::vector<Real>>("length_factor")),
    _sigma_f_squared(getParam<Real>("signal_variance")),
    _sigma_n_squared(getParam<Real>("noise_variance"))
{
}

void
CovarianceFunctionBase::buildHyperParamMap(
    std::unordered_map<std::string, Real> & map,
    std::unordered_map<std::string, std::vector<Real>> & vec_map) const
{
  map["noise_variance"] = _sigma_n_squared;
  map["signal_variance"] = _sigma_f_squared;

  vec_map["length_factor"] = _length_factor;

  buildAdditionalHyperParamMap(map, vec_map);
}

void
CovarianceFunctionBase::loadHyperParamMap(
    std::unordered_map<std::string, Real> & map,
    std::unordered_map<std::string, std::vector<Real>> & vec_map)
{
  _sigma_n_squared = map["noise_variance"];
  _sigma_f_squared = map["signal_variance"];

  _length_factor = vec_map["length_factor"];

  loadAdditionalHyperParamMap(map, vec_map);
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
CovarianceFunctionBase::getTuningData(std::string name,
                                      unsigned int & size,
                                      Real & min,
                                      Real & max) const
{
  if ((name == "noise_variance") || (name == "signal_variance"))
  {
    min = 1e-9;
    max = 1e9;
    size = 1;
  }
  else if (name == "length_factor")
  {
    min = 1e-9;
    max = 1e9;
    size = _length_factor.size();
  }
}
