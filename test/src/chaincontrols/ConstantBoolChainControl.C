//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConstantBoolChainControl.h"

registerMooseObject("MooseTestApp", ConstantBoolChainControl);

InputParameters
ConstantBoolChainControl::validParams()
{
  InputParameters params = ChainControl::validParams();
  params.addClassDescription("Creates a constant bool control data.");
  params.addRequiredParam<bool>("value", "Constant boolean value");
  return params;
}

ConstantBoolChainControl::ConstantBoolChainControl(const InputParameters & parameters)
  : ChainControl(parameters),
    _constant_value(getParam<bool>("value")),
    _value(declareChainControlData<bool>("value"))

{
}

void
ConstantBoolChainControl::execute()
{
  _value = _constant_value;
}
