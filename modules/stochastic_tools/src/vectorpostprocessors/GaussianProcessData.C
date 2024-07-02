//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Stocastic Tools Includes
#include "GaussianProcessData.h"

registerMooseObject("StochasticToolsApp", GaussianProcessData);

InputParameters
GaussianProcessData::validParams()
{
  InputParameters params = GeneralVectorPostprocessor::validParams();
  params += SurrogateModelInterface::validParams();
  params.addClassDescription(
      "Tool for extracting hyperparameter data from gaussian process user object and "
      "storing in VectorPostprocessor vectors.");
  params.addRequiredParam<UserObjectName>("gp_name", "Name of GaussianProcess.");
  return params;
}

GaussianProcessData::GaussianProcessData(const InputParameters & parameters)
  : GeneralVectorPostprocessor(parameters),
    SurrogateModelInterface(this),
    _gp_surrogate(getSurrogateModel<GaussianProcessSurrogate>("gp_name"))
{
}

void
GaussianProcessData::initialize()
{
  const std::unordered_map<std::string, Real> & _hyperparam_map =
      _gp_surrogate.getGP().getHyperParamMap();
  const std::unordered_map<std::string, std::vector<Real>> & _hyperparam_vec_map =
      _gp_surrogate.getGP().getHyperParamVectorMap();

  for (auto iter = _hyperparam_map.begin(); iter != _hyperparam_map.end(); ++iter)
  {
    _hp_vector.push_back(&declareVector(iter->first));
    _hp_vector.back()->push_back(iter->second);
  }
  for (auto iter = _hyperparam_vec_map.begin(); iter != _hyperparam_vec_map.end(); ++iter)
  {
    std::vector<Real> vec = iter->second;
    for (unsigned int ii = 0; ii < vec.size(); ++ii)
    {
      _hp_vector.push_back(&declareVector(iter->first + std::to_string(ii)));
      _hp_vector.back()->push_back(vec[ii]);
    }
  }
}
