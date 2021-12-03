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

registerMooseObject("MooseApp", SideIntegralMaterialProperty);
registerMooseObject("MooseApp", ADSideIntegralMaterialProperty);

template <bool is_ad>
InputParameters
SideIntegralMaterialPropertyTempl<is_ad>::validParams()
{
  InputParameters params = IndexableProperty<SideIntegralPostprocessor, is_ad>::validParams();
  params.addClassDescription("Compute the integral of a scalar material property component over "
                             "the domain.");
  return params;
}

template <bool is_ad>
SideIntegralMaterialPropertyTempl<is_ad>::SideIntegralMaterialPropertyTempl(
    const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters), _prop(this)
{
}

template <bool is_ad>
void
SideIntegralMaterialPropertyTempl<is_ad>::initialSetup()
{
  // check if the material property type and number of supplied components match
  _prop.check();
}

template <bool is_ad>
Real
SideIntegralMaterialPropertyTempl<is_ad>::computeQpIntegral()
{
  return MetaPhysicL::raw_value(_prop[_qp]);
}

template class SideIntegralMaterialPropertyTempl<false>;
template class SideIntegralMaterialPropertyTempl<true>;
