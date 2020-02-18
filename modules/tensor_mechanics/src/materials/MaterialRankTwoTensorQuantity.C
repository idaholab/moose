//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialRankTwoTensorQuantity.h"
#include "RankTwoScalarTools.h"

registerMooseObject("TensorMechanicsApp", MaterialRankTwoTensorQuantity);

defineLegacyParams(MaterialRankTwoTensorQuantity);

InputParameters
MaterialRankTwoTensorQuantity::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Access a component of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addParam<std::string>("calculation_name",
                               "Optional parameter that allows the user to define "
                               "multiple mechanics material systems on the same "
                               "block, i.e. for multiple phases");
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

MaterialRankTwoTensorQuantity::MaterialRankTwoTensorQuantity(const InputParameters & parameters)
  : Material(parameters),
    _tensor(getMaterialProperty<RankTwoTensor>("rank_two_tensor")),
    _calculation_name(isParamValid("calculation_name") ? getParam<std::string>("calculation_name")
                                                       : ""),
    _calculation(declareProperty<Real>(_calculation_name)),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j"))
{
}

void
MaterialRankTwoTensorQuantity::initQpStatefulProperties()
{
  _calculation[_qp] = 0.0;
}

void
MaterialRankTwoTensorQuantity::computeQpProperties()
{
  unsigned int qp = _qp;

  _calculation[_qp] = RankTwoScalarTools::component(_tensor[qp], _i, _j);
}
