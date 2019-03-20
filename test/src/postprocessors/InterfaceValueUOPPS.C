//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceValueUOPPS.h"
#include "InterfaceValueAverageUO.h"

registerMooseObject("MooseTestApp", InterfaceValueUOPPS);

template <>
InputParameters
validParams<InterfaceValueUOPPS>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addRequiredParam<UserObjectName>("user_object", "The name of the user object");
  params.addClassDescription(
      "Test Interface User Object postprocessor getting value from InterfaceValueAverageUO");
  return params;
}

InterfaceValueUOPPS::InterfaceValueUOPPS(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _uo(getUserObject<InterfaceValueAverageUO>("user_object")),
    _value_pp(0.)
{
}

InterfaceValueUOPPS::~InterfaceValueUOPPS() {}

void
InterfaceValueUOPPS::initialize()
{
  _value_pp = 0;
}

void
InterfaceValueUOPPS::execute()
{
  _value_pp = _uo.getValue();
}

Real
InterfaceValueUOPPS::getValue()
{
  return _value_pp;
}
