//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DefaultMatPropConsumerKernel.h"

registerMooseObject("MooseTestApp", DefaultMatPropConsumerKernel);
registerMooseObject("MooseTestApp", ADDefaultMatPropConsumerKernel);

template <bool is_ad>
InputParameters
DefaultMatPropConsumerKernelTempl<is_ad>::validParams()
{
  InputParameters params = GenericKernel<is_ad>::validParams();
  params.addParam<MaterialPropertyName>("mat_prop", "prop", "Material property name to fetch");
  return params;
}

template <bool is_ad>
DefaultMatPropConsumerKernelTempl<is_ad>::DefaultMatPropConsumerKernelTempl(
    const InputParameters & parameters)
  : DerivativeMaterialInterface<GenericKernel<is_ad>>(parameters),
    _prop(this->template getDefaultMaterialProperty<Real, is_ad>("mat_prop"))
{
}
