//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InterfaceUOPPS.h"
#include "InterfaceUO.h"

registerMooseObject("MooseTestApp", InterfaceUOPPS);

template <>
InputParameters
validParams<InterfaceUOPPS>()
{
  InputParameters params = validParams<InterfacePostprocessor>();
  params.addRequiredParam<UserObjectName>("user_object", "The name of the user object");
  params.suppressParameter<std::vector<BoundaryName>>("boundary");
  params.addClassDescription(
      "Test Interfae User Object postprocessor getting value from InterfaceUO");
  return params;
}

InterfaceUOPPS::InterfaceUOPPS(const InputParameters & parameters)
  : InterfacePostprocessor(parameters),
    _uo(getUserObject<InterfaceUO>("user_object")),
    _value_pp(0.)
{
}

InterfaceUOPPS::~InterfaceUOPPS() {}

void
InterfaceUOPPS::initialize()
{
  _value_pp = 0;
}

void
InterfaceUOPPS::execute()
{
  _value_pp = _uo.getValue();
}

Real
InterfaceUOPPS::getValue()
{
  return _value_pp;
}
