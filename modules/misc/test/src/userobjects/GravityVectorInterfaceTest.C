//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GravityVectorInterfaceTest.h"

registerMooseObject("MiscTestApp", GravityVectorInterfaceTest);

InputParameters
GravityVectorInterfaceTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += GravityVectorInterface::validParams();
  return params;
}

GravityVectorInterfaceTest::GravityVectorInterfaceTest(const InputParameters & parameters)
  : GeneralUserObject(parameters), GravityVectorInterface(this)
{
}
