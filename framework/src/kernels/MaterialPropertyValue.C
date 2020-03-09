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

InputParameters
MaterialPropertyValue::validParams()
{
  InputParameters params = KernelValue::validParams();
  params.addClassDescription(
      "Residual term (u - prop) to set variable u equal to a given material property prop");
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "Name of material property to be used in the kernel");
  params.addParam<bool>(
      "positive", true, "If the kernel is positive, this is true, if negative, it is false");
  return params;
}

MaterialPropertyValue::MaterialPropertyValue(const InputParameters & parameters)
  : KernelValue(parameters),
    _kernel_sign(getParam<bool>("positive") ? 1.0 : -1.0),
    _prop(getMaterialProperty<Real>("prop_name"))
{
}

Real
MaterialPropertyValue::precomputeQpResidual()
{
  return _kernel_sign * (_prop[_qp] - _u[_qp]);
}

Real
MaterialPropertyValue::precomputeQpJacobian()
{
  return -_kernel_sign * _phi[_j][_qp];
}
