//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CZMRealVectorCartesianComponent.h"

registerMooseObject("TensorMechanicsApp", CZMRealVectorCartesianComponent);

InputParameters
CZMRealVectorCartesianComponent::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("Access a component of a RealVectorValue defined on a cohesive zone");
  params.addRequiredParam<std::string>("real_vector_value", "The vector material name");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "Name of the material property computed by this model");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index", "index >= 0 & index <= 2", "The vector component output (0, 1, 2)");
  params.addParam<std::string>("base_name", "Material property base name");
  return params;
}

CZMRealVectorCartesianComponent::CZMRealVectorCartesianComponent(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _base_name(isParamValid("base_name") && !getParam<std::string>("base_name").empty()
                   ? getParam<std::string>("base_name") + "_"
                   : ""),
    _property(declarePropertyByName<Real>(getParam<MaterialPropertyName>("property_name"))),
    _vector(getMaterialPropertyByName<RealVectorValue>(_base_name +
                                                       getParam<std::string>("real_vector_value"))),
    _index(getParam<unsigned int>("index"))
{
}

void
CZMRealVectorCartesianComponent::computeQpProperties()
{
  _property[_qp] = _vector[_qp](_index);
}
