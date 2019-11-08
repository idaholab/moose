//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OldMaterialAux.h"

registerMooseObject("MooseTestApp", OldMaterialAux);

InputParameters
OldMaterialAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>(
      "property_name", "The name of the material property to capture old and older values from");
  return params;
}

OldMaterialAux::OldMaterialAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _old(getMaterialPropertyOld<Real>("property_name")),
    _older(getMaterialPropertyOlder<Real>("property_name"))
{
}

Real
OldMaterialAux::computeValue()
{
  return _old[_qp] + _older[_qp];
}
