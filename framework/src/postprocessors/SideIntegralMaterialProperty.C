//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SideIntegralMaterialProperty.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", SideIntegralMaterialRealProperty);
registerMooseObject("MooseApp", ADSideIntegralMaterialRealProperty);
registerMooseObject("MooseApp", SideIntegralMaterialRealVectorValueProperty);
registerMooseObject("MooseApp", ADSideIntegralMaterialRealVectorValueProperty);
registerMooseObject("MooseApp", SideIntegralMaterialRankTwoTensorProperty);
registerMooseObject("MooseApp", ADSideIntegralMaterialRankTwoTensorProperty);
registerMooseObject("MooseApp", SideIntegralMaterialRankThreeTensorProperty);
registerMooseObject("MooseApp", ADSideIntegralMaterialRankThreeTensorProperty);
registerMooseObject("MooseApp", SideIntegralMaterialRankFourTensorProperty);
registerMooseObject("MooseApp", ADSideIntegralMaterialRankFourTensorProperty);

template <typename T, bool is_ad>
InputParameters
SideIntegralMaterialPropertyTempl<T, is_ad>::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params += ComponentUtils<T>::validParams();
  params.addRequiredParam<MaterialPropertyName>("mat_prop", "The name of the material property");
  params.addClassDescription(
      "Compute the integral of the material property over the domain (scalar/component)");
  return params;
}

template <typename T, bool is_ad>
SideIntegralMaterialPropertyTempl<T, is_ad>::SideIntegralMaterialPropertyTempl(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    ComponentUtils<T>(parameters),
    _prop(getGenericMaterialProperty<T, is_ad>("mat_prop"))
{
}

template <typename T, bool is_ad>
Real
SideIntegralMaterialPropertyTempl<T, is_ad>::computeQpIntegral()
{
  return this->getComponent(MetaPhysicL::raw_value(_prop[_qp]));
}

template class SideIntegralMaterialPropertyTempl<Real, false>;
template class SideIntegralMaterialPropertyTempl<Real, true>;
template class SideIntegralMaterialPropertyTempl<RealVectorValue, false>;
template class SideIntegralMaterialPropertyTempl<RealVectorValue, true>;
template class SideIntegralMaterialPropertyTempl<RankTwoTensor, false>;
template class SideIntegralMaterialPropertyTempl<RankTwoTensor, true>;
template class SideIntegralMaterialPropertyTempl<RankThreeTensor, false>;
template class SideIntegralMaterialPropertyTempl<RankThreeTensor, true>;
template class SideIntegralMaterialPropertyTempl<RankFourTensor, false>;
template class SideIntegralMaterialPropertyTempl<RankFourTensor, true>;
