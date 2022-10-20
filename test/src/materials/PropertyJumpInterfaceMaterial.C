//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PropertyJumpInterfaceMaterial.h"

registerMooseObject("MooseTestApp", PropertyJumpInterfaceMaterial);

InputParameters
PropertyJumpInterfaceMaterial::validParams()
{
  InputParameters params = InterfaceMaterial::validParams();
  params.addClassDescription("Calculates a property's jump value across an interface.");
  params.addRequiredParam<MaterialPropertyName>("property", "Name of the property");
  return params;
}

PropertyJumpInterfaceMaterial::PropertyJumpInterfaceMaterial(const InputParameters & parameters)
  : InterfaceMaterial(parameters),
    _property(getADMaterialProperty<Real>("property")),
    _neighbor_property(getNeighborADMaterialProperty<Real>("property")),
    _jump(declareADProperty<Real>("jump"))
{
}

void
PropertyJumpInterfaceMaterial::computeQpProperties()
{
  _jump[_qp] = _property[_qp] - _neighbor_property[_qp];
}
