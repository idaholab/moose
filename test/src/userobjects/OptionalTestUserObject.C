//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OptionalTestUserObject.h"
#include "Function.h"

registerMooseObject("MooseTestApp", OptionalTestUserObject);

InputParameters
OptionalTestUserObject::validParams()
{
  InputParameters params = ElementUserObject::validParams();
  params.addRequiredParam<MaterialPropertyName>("prop", "optional non-AD property");
  params.addRequiredParam<MaterialPropertyName>("adprop", "optional AD property");
  params.addRequiredParam<bool>("expect", "expect non-AD property to exist");
  params.addRequiredParam<bool>("adexpect", "expect AD property to exist");
  params.addRequiredParam<FunctionName>("gold_function",
                                        "Function to compare the property values to");
  params.addRequiredParam<FunctionName>("ad_gold_function",
                                        "Function to compare the AD property values to");
  return params;
}

OptionalTestUserObject::OptionalTestUserObject(const InputParameters & parameters)
  : ElementUserObject(parameters),
    _prop(getOptionalMaterialProperty<Real>("prop")),
    _adprop(getOptionalADMaterialProperty<Real>("adprop")),
    _prop_old(getOptionalMaterialPropertyOld<Real>("prop")),
    _prop_older(getOptionalMaterialPropertyOlder<Real>("prop")),
    _expect(getParam<bool>("expect")),
    _adexpect(getParam<bool>("adexpect")),
    _expect_old(_expect || _adexpect),
    _func(getFunction("gold_function")),
    _adfunc(getFunction("ad_gold_function"))
{
}

void
OptionalTestUserObject::initialSetup()
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
}

void
OptionalTestUserObject::execute()
{
  for (unsigned int qp = 0; qp < _qrule->n_points(); ++qp)
  {
    if (_prop && _prop[qp] != _func.value(_t, _q_point[qp]))
      mooseError("Property does not match gold function");
    if (_adprop && MetaPhysicL::raw_value(_adprop[qp]) != _adfunc.value(_t, _q_point[qp]))
      mooseError("AD property does not match gold function");

    if (_t >= 1.0 && _prop_old && _prop_old[qp] != _func.value(_t - 1.0, _q_point[qp]))
      mooseError("Property old does not match gold function");
    if (_t >= 2.0 && _prop_older && _prop_older[qp] != _func.value(_t - 2.0, _q_point[qp]))
      mooseError("Property older does not match gold function");
  }
}
