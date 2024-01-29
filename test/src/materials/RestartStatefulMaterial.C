//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RestartStatefulMaterial.h"

registerMooseObject("MooseTestApp", RestartStatefulMaterial);

InputParameters
RestartStatefulMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addParam<std::vector<std::string>>(
      "real_names", {}, "The name of the real properties to declare");
  params.addParam<std::vector<Real>>(
      "real_values", {}, "The values to set for the real properties");
  params.addParam<std::vector<std::string>>(
      "real_stateful_names", {}, "The name of the real properties to request as old");
  params.addParam<std::vector<std::string>>(
      "real_older_stateful_names", {}, "The name of the real properties to request as older");
  params.addParam<std::vector<std::string>>(
      "int_names", {}, "The name of the int properties to declare");
  return params;
}

RestartStatefulMaterial::RestartStatefulMaterial(const InputParameters & parameters)
  : Material(parameters),
    _real_names(getParam<std::vector<std::string>>("real_names")),
    _real_values(getParam<std::vector<Real>>("real_values"))
{
  if (_real_values.size() > 0 && (_real_names.size() != _real_values.size()))
    paramError("real_names", "Size mismatch");
  for (const auto & name : _real_names)
    declareProperty<Real>(name);
  for (const auto & name : getParam<std::vector<std::string>>("real_stateful_names"))
    getMaterialPropertyOld<Real>(name);
  for (const auto & name : getParam<std::vector<std::string>>("real_older_stateful_names"))
    getMaterialPropertyOlder<Real>(name);
  for (const auto & name : getParam<std::vector<std::string>>("int_names"))
  {
    declareProperty<int>(name);
    getMaterialPropertyOld<int>(name);
  }
}

void
RestartStatefulMaterial::initQpStatefulProperties()
{
  if (_real_values.size())
    for (const auto i : index_range(_real_props))
      (*_real_props[i])[_qp] = _real_values[i];
}

void
RestartStatefulMaterial::computeQpProperties()
{
  if (_real_values.size())
    for (const auto i : index_range(_real_props))
      (*_real_props[i])[_qp] = _real_values[i];
}
