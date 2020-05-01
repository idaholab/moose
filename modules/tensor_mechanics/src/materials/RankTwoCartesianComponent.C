//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoCartesianComponent.h"
#include "RankTwoScalarTools.h"

registerMooseObject("TensorMechanicsApp", RankTwoCartesianComponent);

InputParameters
RankTwoCartesianComponent::validParams()
{
  InputParameters params = Material::validParams();
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

RankTwoCartesianComponent::RankTwoCartesianComponent(const InputParameters & parameters)
  : Material(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _property_name(isParamValid("property_name") ? getParam<std::string>("property_name") : ""),
    _property(declareProperty<Real>(_property_name)),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j"))
{
}

void
RankTwoCartesianComponent::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

void
RankTwoCartesianComponent::computeQpProperties()
{
  _property[_qp] = RankTwoScalarTools::component(_tensor[_qp], _i, _j);
}
