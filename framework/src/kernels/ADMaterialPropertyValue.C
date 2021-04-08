//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADMaterialPropertyValue.h"

registerMooseObject("MooseApp", ADMaterialPropertyValue);

InputParameters
ADMaterialPropertyValue::validParams()
{
  InputParameters params = ADKernelValue::validParams();
  params.addClassDescription(
      "Residual term (u - prop) to set variable u equal to a given material property prop");
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "Name of material property to be used in the kernel");
  params.addParam<bool>(
      "positive", true, "If the kernel is positive, this is true, if negative, it is false");
  return params;
}

ADMaterialPropertyValue::ADMaterialPropertyValue(const InputParameters & parameters)
  : ADKernelValue(parameters),
    _kernel_sign(getParam<bool>("positive") ? 1.0 : -1.0),
    _prop(getADMaterialProperty<Real>("prop_name"))
{
}

ADReal
ADMaterialPropertyValue::precomputeQpResidual()
{
  return _kernel_sign * (_prop[_qp] - _u[_qp]);
}
