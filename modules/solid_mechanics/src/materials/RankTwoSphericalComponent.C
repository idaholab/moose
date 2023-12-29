//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RankTwoSphericalComponent.h"
#include "RankTwoScalarTools.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("TensorMechanicsApp", RankTwoSphericalComponent);
registerMooseObject("TensorMechanicsApp", ADRankTwoSphericalComponent);

template <bool is_ad>
InputParameters
RankTwoSphericalComponentTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Compute components of a rank-2 tensor in a spherical coordinate system");
  params.addRequiredParam<MaterialPropertyName>("rank_two_tensor",
                                                "The rank two material tensor name");
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "Name of the material property computed by this model");
  MooseEnum sphericalTypes("HoopStress RadialStress");
  params.addParam<MooseEnum>(
      "spherical_component", sphericalTypes, "Type of spherical scalar output");
  params.addParam<Point>("spherical_center_point",
                         "Center point of the spherical coordinate system.");
  return params;
}

template <bool is_ad>
RankTwoSphericalComponentTempl<is_ad>::RankTwoSphericalComponentTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _tensor(getGenericMaterialProperty<RankTwoTensor, is_ad>("rank_two_tensor")),
    _property(declareGenericProperty<Real, is_ad>("property_name")),
    _spherical_component(getParam<MooseEnum>("spherical_component")
                             .template getEnum<RankTwoScalarTools::SphericalComponent>()),
    _center(isParamValid("spherical_center_point") ? getParam<Point>("spherical_center_point")
                                                   : Point(0, 0, 0))
{
}

template <bool is_ad>
void
RankTwoSphericalComponentTempl<is_ad>::initQpStatefulProperties()
{
  _property[_qp] = 0.0;
}

template <bool is_ad>
void
RankTwoSphericalComponentTempl<is_ad>::computeQpProperties()
{
  Point dummy_direction;

  _property[_qp] = RankTwoScalarTools::getSphericalComponent(MetaPhysicL::raw_value(_tensor[_qp]),
                                                             _spherical_component,
                                                             _center,
                                                             _q_point[_qp],
                                                             dummy_direction);
}

template class RankTwoSphericalComponentTempl<false>;
template class RankTwoSphericalComponentTempl<true>;
