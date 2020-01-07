//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarUOIC.h"
#include "MTUserObject.h"

registerMooseObject("MooseTestApp", ScalarUOIC);

InputParameters
ScalarUOIC::validParams()
{
  InputParameters params = ScalarInitialCondition::validParams();
  params.addRequiredParam<UserObjectName>("user_object",
                                          "The MTUserObject to be coupled into this IC");
  return params;
}

ScalarUOIC::ScalarUOIC(const InputParameters & parameters)
  : ScalarInitialCondition(parameters), _uo(getUserObject<MTUserObject>("user_object"))
{
}

Real
ScalarUOIC::value()
{
  return _uo.doSomething();
}
