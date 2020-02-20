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
  params.addClassDescription("Compute a scalar property of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addRequiredParam<std::string>("property_name",
                                       "Name of the material property computed by this model");
  params.addParam<MooseEnum>(
      "invariant", RankTwoScalarTools::cartesianOptions(), "Type of scalar output");
  return params;
}

template <bool is_ad>
RankTwoCartesianComponentTempl<is_ad>::RankTwoCartesianComponentTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _property_name(
        isParamValid("property_name") ? this->template getParam<std::string>("property_name") : ""),
    _property(declareGenericProperty<Real, is_ad>(_property_name)),
    _invariant(this->template getParam<MooseEnum>("invariant"))
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
  _property[_qp] =
      RankTwoScalarTools::getCartesianComponent(MetaPhysicL::raw_value(_tensor[_qp]), _invariant);
}

template class RankTwoCartesianComponentTempl<false>;
template class RankTwoCartesianComponentTempl<true>;
