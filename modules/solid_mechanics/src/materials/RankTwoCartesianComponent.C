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

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", RankTwoCartesianComponent);
registerMooseObject("TensorMechanicsApp", ADRankTwoCartesianComponent);

template <bool is_ad>
InputParameters
RankTwoCartesianComponentTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Access a component of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material property tensor name");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "Name of the material property computed by this model");
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

template <bool is_ad>
RankTwoCartesianComponentTempl<is_ad>::RankTwoCartesianComponentTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _property(declareGenericProperty<Real, is_ad>("property_name")),
    _i(getParam<unsigned int>("index_i")),
    _j(getParam<unsigned int>("index_j"))
{
}

template <bool is_ad>
void
RankTwoCartesianComponentTempl<is_ad>::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

template <bool is_ad>
void
RankTwoCartesianComponentTempl<is_ad>::computeQpProperties()
{
  _property[_qp] = RankTwoScalarTools::component(MetaPhysicL::raw_value(_tensor[_qp]), _i, _j);
}

template class RankTwoCartesianComponentTempl<false>;
template class RankTwoCartesianComponentTempl<true>;
