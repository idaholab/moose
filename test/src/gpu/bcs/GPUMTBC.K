//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GPUMTBC.h"

registerMooseObject("MooseTestApp", GPUMTBC);

InputParameters
GPUMTBC::validParams()
{
  InputParameters params = GPUIntegratedBC::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "the name of the material property we are going to use");
  params.addRequiredParam<Real>("grad", "the value of the gradient");
  return params;
}

GPUMTBC::GPUMTBC(const InputParameters & parameters)
  : GPUIntegratedBC(parameters),
    _value(getParam<Real>("grad")),
    _mat(getGPUMaterialProperty<Real>("prop_name"))
{
}
