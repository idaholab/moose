//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InsideValuePPS.h"
#include "InsideUserObject.h"

registerMooseObject("MooseTestApp", InsideValuePPS);

InputParameters
InsideValuePPS::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addRequiredParam<UserObjectName>("user_object", "The name of the user object");

  return params;
}

InsideValuePPS::InsideValuePPS(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _uo(getUserObject<InsideUserObject>("user_object")),
    _value(0.)
{
}

InsideValuePPS::~InsideValuePPS() {}

void
InsideValuePPS::initialize()
{
  _value = 0;
}

void
InsideValuePPS::execute()
{
  _value = _uo.getValue();
}

Real
InsideValuePPS::getValue()
{
  return _value;
}
