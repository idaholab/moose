//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details

#include "OldMaterialProperty.h"

registerMooseObject("MooseTestApp", OldMaterialProperty);

InputParameters
OldMaterialProperty::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("property", "Property that receives the old value");
  params.addRequiredParam<MaterialPropertyName>("coupled_property",
                                                "Property whose old value is copied");
  return params;
}

OldMaterialProperty::OldMaterialProperty(const InputParameters & parameters)
  : Material(parameters),
    _property(declareProperty<Real>("property")),
    _old_property(getMaterialPropertyOld<Real>("coupled_property"))
{
}

void
OldMaterialProperty::computeQpProperties()
{
  _property[_qp] = _old_property[_qp];
}
