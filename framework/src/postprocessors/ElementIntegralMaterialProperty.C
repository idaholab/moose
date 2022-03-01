//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIntegralMaterialProperty.h"

#include "metaphysicl/raw_type.h"

registerMooseObject("MooseApp", ElementIntegralMaterialProperty);
registerMooseObject("MooseApp", ADElementIntegralMaterialProperty);

template <bool is_ad>
InputParameters
ElementIntegralMaterialPropertyTempl<is_ad>::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredParam<MaterialPropertyName>("mat_prop", "The name of the material property");
  params.addClassDescription("Compute the integral of the material property over the domain");
  return params;
}

template <bool is_ad>
ElementIntegralMaterialPropertyTempl<is_ad>::ElementIntegralMaterialPropertyTempl(
    const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _scalar(getGenericMaterialProperty<Real, is_ad>("mat_prop"))
{
}

template <bool is_ad>
Real
ElementIntegralMaterialPropertyTempl<is_ad>::computeQpIntegral()
{
  return MetaPhysicL::raw_value(_scalar[_qp]);
}

template class ElementIntegralMaterialPropertyTempl<false>;
template class ElementIntegralMaterialPropertyTempl<true>;
