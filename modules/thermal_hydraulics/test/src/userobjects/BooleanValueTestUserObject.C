//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BooleanValueTestUserObject.h"

registerMooseObject("ThermalHydraulicsTestApp", BooleanValueTestUserObject);

InputParameters
BooleanValueTestUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addRequiredParam<bool>("value", "A controllable boolean value");
  params.declareControllable("value");
  params.addClassDescription("Test user object that holds a controllable boolean parameter");
  return params;
}

BooleanValueTestUserObject::BooleanValueTestUserObject(const InputParameters & params)
  : GeneralUserObject(params)
{
}
