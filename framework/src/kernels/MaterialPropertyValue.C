//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MaterialPropertyValue.h"

registerMooseObject("MooseApp", MaterialPropertyValue);
registerMooseObject("MooseApp", ADMaterialPropertyValue);

template <bool is_ad>
InputParameters
MaterialPropertyValueTempl<is_ad>::validParams()
{
  InputParameters params = KernelValueParent<is_ad>::validParams();
  params.addClassDescription(
      "Residual term (u - prop) to set variable u equal to a given material property prop");
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "Name of material property to be used in the kernel");
  params.addParam<bool>(
      "positive", true, "If the kernel is positive, this is true, if negative, it is false");
  return params;
}

template <bool is_ad>
MaterialPropertyValueTempl<is_ad>::MaterialPropertyValueTempl(const InputParameters & parameters)
  : KernelValueParent<is_ad>(parameters),
    _kernel_sign(this->template getParam<bool>("positive") ? 1.0 : -1.0),
    _prop(this->template getGenericMaterialProperty<Real, is_ad>("prop_name"))
{
}

template <>
Real
MaterialPropertyValueTempl<false>::precomputeQpResidual()
{
  return _kernel_sign * (_prop[_qp] - _u[_qp]);
}

template <>
ADReal
MaterialPropertyValueTempl<true>::precomputeQpResidual()
{
  return _kernel_sign * (_prop[_qp] - _u[_qp]);
}

template <>
Real
MaterialPropertyValueTempl<false>::precomputeQpJacobian()
{
  return -_kernel_sign * _phi[_j][_qp];
}

template <>
Real
MaterialPropertyValueTempl<true>::precomputeQpJacobian()
{
  mooseError("This should never get called");
}

template class MaterialPropertyValueTempl<false>;
template class MaterialPropertyValueTempl<true>;
