//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "DefaultMatPropConsumerKernel.h"

template <>
InputParameters
validParams<DefaultMatPropConsumerKernel>()
{
  InputParameters params = validParams<Kernel>();
  params.addParam<MaterialPropertyName>("mat_prop", "prop", "Material property name to fetch");
  return params;
}

DefaultMatPropConsumerKernel::DefaultMatPropConsumerKernel(const InputParameters & parameters)
  : DerivativeMaterialInterface<Kernel>(parameters),
    _prop(getDefaultMaterialProperty<Real>("mat_prop"))
{
}
