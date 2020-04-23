//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADRankTwoCartesianComponent.h"
#include "RankTwoScalarTools.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", ADRankTwoCartesianComponent);

InputParameters
ADRankTwoCartesianComponent::validParams()
{
  InputParameters params = ADMaterial::validParams();
  params.addClassDescription("Access a component of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addRequiredParam<std::string>("property_name",
                                       "Name of the material property computed by this model");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_i",
      "index_i >= 0 & index_i <= 2",
      "The index i of ij for the tensor to output (0, 1, 2)");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "index_j",
      "index_j >= 0 & index_j <= 2",
      "The index j of ij for the tensor to output (0, 1, 2)");
  return params;
}

ADRankTwoCartesianComponent::ADRankTwoCartesianComponent(const InputParameters & parameters)
  : ADMaterial(parameters),
    _tensor(getADMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _property_name(
        isParamValid("property_name") ? this->template getParam<std::string>("property_name") : ""),
    _property(declareADProperty<Real>(_property_name)),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j"))
{
}

void
ADRankTwoCartesianComponent::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

void
ADRankTwoCartesianComponent::computeQpProperties()
{
  _property[_qp] = RankTwoScalarTools::component(MetaPhysicL::raw_value(_tensor[_qp]), _i, _j);
}
