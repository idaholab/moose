//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRankTwoDirectionalComponent.h"
#include "RankTwoScalarTools.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", ADRankTwoDirectionalComponent);

InputParameters
ADRankTwoDirectionalComponent::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription("Compute a Direction scalar property of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addRequiredParam<std::string>("property_name",
                                       "Name of the material property computed by this model");
  params.addParam<MooseEnum>(
      "invariant", RankTwoScalarTools::directionOption(), "Type of scalar output");
  params.addRequiredParam<Point>("direction", "Direction to calculate component in.");
  return params;
}

ADRankTwoDirectionalComponent::ADRankTwoDirectionalComponent(const InputParameters & parameters)
  : ADMaterial(parameters),
    _tensor(getADMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _property_name(
        isParamValid("property_name") ? this->template getParam<std::string>("property_name") : ""),
    _property(declareADProperty<Real>(_property_name)),
    _invariant(this->template getParam<MooseEnum>("invariant")),
    _direction(this->template getParam<Point>("direction"))
{
}

void
ADRankTwoDirectionalComponent::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

void
ADRankTwoDirectionalComponent::computeQpProperties()
{
  _property[_qp] = RankTwoScalarTools::getDirectionalComponent(
      MetaPhysicL::raw_value(_tensor[_qp]), _invariant, _direction);
}
