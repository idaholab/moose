//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptionalTestAux.h"

registerMooseObject("MooseTestApp", OptionalTestAux);

InputParameters
OptionalTestAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop", "optional non-AD property");
  params.addRequiredParam<MaterialPropertyName>("adprop", "optional AD property");
  params.addRequiredParam<bool>("expect", "expect non-AD property to exist");
  params.addRequiredParam<bool>("adexpect", "expect AD property to exist");
  return params;
}

OptionalTestAux::OptionalTestAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _prop(getOptionalMaterialProperty<Real>("prop")),
    _adprop(getOptionalADMaterialProperty<Real>("adprop")),
    _expect(getParam<bool>("expect")),
    _adexpect(getParam<bool>("adexpect"))
{
}

Real
OptionalTestAux::computeValue()
{
  if (_expect && !_prop)
    mooseError("Non-AD property expected but not found");
  if (!_expect && _prop)
    mooseError("Non-AD property not expected but found");

  if (_adexpect && !_adprop)
    mooseError("AD property expected but not found");
  if (!_adexpect && _adprop)
    mooseError("AD property not expected but found");

  return (_prop ? _prop[_qp] : 0.0) + (_adprop ? MetaPhysicL::raw_value(_adprop[_qp]) : 0.0);
}
