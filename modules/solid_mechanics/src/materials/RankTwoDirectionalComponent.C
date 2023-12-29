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

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", RankTwoDirectionalComponent);
registerMooseObject("TensorMechanicsApp", ADRankTwoDirectionalComponent);

template <bool is_ad>
InputParameters
RankTwoDirectionalComponentTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription("Compute a Direction scalar property of a RankTwoTensor");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material property tensor name");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "Name of the material property computed by this model");
  params.addRequiredParam<Point>("direction", "Direction to calculate component in.");
  return params;
}

template <bool is_ad>
RankTwoDirectionalComponentTempl<is_ad>::RankTwoDirectionalComponentTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _property(declareGenericProperty<Real, is_ad>("property_name")),
    _direction(getParam<Point>("direction"))
{
}

template <bool is_ad>
void
RankTwoDirectionalComponentTempl<is_ad>::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

template <bool is_ad>
void
RankTwoDirectionalComponentTempl<is_ad>::computeQpProperties()
{
  _property[_qp] =
      RankTwoScalarTools::getDirectionalComponent(MetaPhysicL::raw_value(_tensor[_qp]), _direction);
}

template class RankTwoDirectionalComponentTempl<false>;
template class RankTwoDirectionalComponentTempl<true>;
