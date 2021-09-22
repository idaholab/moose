//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoCylindricalComponent.h"
#include "RankTwoScalarTools.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", RankTwoCylindricalComponent);
registerMooseObject("TensorMechanicsApp", ADRankTwoCylindricalComponent);

template <bool is_ad>
InputParameters
RankTwoCylindricalComponentTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Compute components of a rank-2 tensor in a cylindrical coordinate system");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material property tensor name");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "Name of the material property computed by this model");
  MooseEnum cylindricalTypes("AxialStress HoopStress RadialStress");
  params.addParam<MooseEnum>(
      "cylindrical_component", cylindricalTypes, "Type of cylindrical scalar output");
  params.addParam<Point>(
      "cylindrical_axis_point1",
      "Start point for determining axis of rotation for cylindrical stress/strain components");
  params.addParam<Point>(
      "cylindrical_axis_point2",
      "End point for determining axis of rotation for cylindrical stress/strain components");
  return params;
}

template <bool is_ad>
RankTwoCylindricalComponentTempl<is_ad>::RankTwoCylindricalComponentTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _property(declareGenericProperty<Real, is_ad>("property_name")),
    _cylindrical_component(getParam<MooseEnum>("cylindrical_component")
                               .template getEnum<RankTwoScalarTools::CylindricalComponent>()),
    _cylindrical_axis_point1(isParamValid("cylindrical_axis_point1")
                                 ? getParam<Point>("cylindrical_axis_point1")
                                 : Point(0, 0, 0)),
    _cylindrical_axis_point2(isParamValid("cylindrical_axis_point2")
                                 ? getParam<Point>("cylindrical_axis_point2")
                                 : Point(0, 1, 0))
{
}

template <bool is_ad>
void
RankTwoCylindricalComponentTempl<is_ad>::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

template <bool is_ad>
void
RankTwoCylindricalComponentTempl<is_ad>::computeQpProperties()
{
  Point dummy_direction;

  _property[_qp] = RankTwoScalarTools::getCylindricalComponent(MetaPhysicL::raw_value(_tensor[_qp]),
                                                               _cylindrical_component,
                                                               _cylindrical_axis_point1,
                                                               _cylindrical_axis_point2,
                                                               _q_point[_qp],
                                                               dummy_direction);
}

template class RankTwoCylindricalComponentTempl<false>;
template class RankTwoCylindricalComponentTempl<true>;
