//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptionalTestMaterial.h"

registerMooseObject("MooseTestApp", OptionalTestMaterial);

InputParameters
OptionalTestMaterial::validParams()
{
  InputParameters params = Material::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop", "optional non-AD property");
  params.addRequiredParam<MaterialPropertyName>("adprop", "optional AD property");
  params.addRequiredParam<bool>("expect", "expect non-AD property to exist");
  params.addRequiredParam<bool>("adexpect", "expect AD property to exist");
  return params;
}

OptionalTestMaterial::OptionalTestMaterial(const InputParameters & parameters)
  : Material(parameters),
    _prop(getOptionalMaterialProperty<Real>("prop")),
    _adprop(getOptionalADMaterialProperty<Real>("adprop")),
    _prop_old(getOptionalMaterialPropertyOld<Real>("prop")),
    _prop_older(getOptionalMaterialPropertyOlder<Real>("prop")),
    _expect(getParam<bool>("expect")),
    _adexpect(getParam<bool>("adexpect")),
    _expect_old(_expect || _adexpect),
    _mirror(declareProperty<Real>("mirror"))
{
}

void
OptionalTestMaterial::computeQpProperties()
{
  if (_expect && !_prop)
    mooseError("Non-AD property expected but not found");
  if (!_expect && _prop)
    mooseError("Non-AD property not expected but found");

  if (_adexpect && !_adprop)
    mooseError("AD property expected but not found");
  if (!_adexpect && _adprop)
    mooseError("AD property not expected but found");

  if (_expect_old && !_prop_old)
    mooseError("Old property expected but not found");
  if (!_expect_old && _prop_old)
    mooseError("Old property not expected but found");

  if (_expect_old && !_prop_older)
    mooseError("Old property expected but not found");
  if (!_expect_old && _prop_older)
    mooseError("Old property not expected but found");

  _mirror[_qp] = (_prop ? _prop[_qp] : 0.0) + (_prop_old ? _prop_old[_qp] : 0.0) +
                 (_prop_older ? _prop_older[_qp] : 0.0) +
                 (_adprop ? MetaPhysicL::raw_value(_adprop[_qp]) : 0.0);
}
