//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AnisotropicRankTwoTensor.h"

#include "Function.h"

registerMooseObject("MooseApp", AnisotropicRankTwoTensor);
registerMooseObject("MooseApp", ADAnisotropicRankTwoTensor);

template <bool is_ad>
InputParameters
AnisotropicRankTwoTensorTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();
  params.addClassDescription(
      "Material object for defining an anisotropic rank two tensor property by specifying the "
      "three principal components and optioonally the bases.");
  params.addRequiredParam<MaterialPropertyName>(
      "value_1", "First principal component of the anisotropic rank two tensor");
  params.addRequiredParam<MaterialPropertyName>(
      "value_2", "Second principal component of the anisotropic rank two tensor");
  params.addRequiredParam<MaterialPropertyName>(
      "value_3", "Third principal component of the anisotropic rank two tensor");
  params.addParam<RealVectorValue>(
      "direction_1",
      RealVectorValue(1, 0, 0),
      "Local basis vector associated with the first principal component");
  params.addParam<RealVectorValue>(
      "direction_2",
      RealVectorValue(0, 1, 0),
      "Local basis vector associated with the second principal component");
  params.addParam<RealVectorValue>(
      "direction_3",
      RealVectorValue(0, 0, 1),
      "Local basis vector associated with the third principal component");
  params.addRequiredParam<MaterialPropertyName>(
      "tensor_name", "Name of the tensor material property to be created");
  return params;
}

template <bool is_ad>
AnisotropicRankTwoTensorTempl<is_ad>::AnisotropicRankTwoTensorTempl(
    const InputParameters & parameters)
  : Material(parameters),
    _prop(declareGenericProperty<RankTwoTensor, is_ad>("tensor_name")),
    _k1(getGenericMaterialProperty<Real, is_ad>("value_1")),
    _k2(getGenericMaterialProperty<Real, is_ad>("value_2")),
    _k3(getGenericMaterialProperty<Real, is_ad>("value_3")),
    _e1(getParam<RealVectorValue>("direction_1")),
    _e2(getParam<RealVectorValue>("direction_2")),
    _e3(getParam<RealVectorValue>("direction_3"))
{
}

template <bool is_ad>
void
AnisotropicRankTwoTensorTempl<is_ad>::computeQpProperties()
{
  _prop[_qp] = 0;
  _prop[_qp] += _k1[_qp] * GenericRankTwoTensor<is_ad>::selfOuterProduct(_e1);
  _prop[_qp] += _k2[_qp] * GenericRankTwoTensor<is_ad>::selfOuterProduct(_e2);
  _prop[_qp] += _k3[_qp] * GenericRankTwoTensor<is_ad>::selfOuterProduct(_e3);
}
