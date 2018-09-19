//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "DotMaterialAux.h"

registerMooseObject("MooseTestApp", DotMaterialAux);

template <>
InputParameters
validParams<DotMaterialAux>()
{
  InputParameters params = validParams<AuxKernel>();
  params.addParam<MaterialPropertyName>(
      "property_name", 1.0, "The name of the material property to get time derivative");
  return params;
}

DotMaterialAux::DotMaterialAux(const InputParameters & parameters)
  : AuxKernel(parameters), _dot(getMaterialPropertyDot("property_name"))
{
}

Real
DotMaterialAux::computeValue()
{
  return _dot[_qp];
}
