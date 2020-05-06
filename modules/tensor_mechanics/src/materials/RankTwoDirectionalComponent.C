//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoDirectionalComponent.h"
#include "RankTwoScalarTools.h"

registerMooseObject("TensorMechanicsApp", RankTwoDirectionalComponent);

InputParameters
RankTwoDirectionalComponent::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a Direction scalar property of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addRequiredParam<std::string>("property_name",
                                       "Name of the material property computed by this model");
  params.addRequiredParam<Point>("direction", "Direction to calculate component in.");
  return params;
}

RankTwoDirectionalComponent::RankTwoDirectionalComponent(const InputParameters & parameters)
  : Material(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _property_name(isParamValid("property_name") ? getParam<std::string>("property_name") : ""),
    _property(declareProperty<Real>(_property_name)),
    _direction(getParam<Point>("direction"))
{
}

void
RankTwoDirectionalComponent::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

void
RankTwoDirectionalComponent::computeQpProperties()
{
  _property[_qp] = RankTwoScalarTools::getDirectionalComponent(_tensor[_qp], _direction);
}
