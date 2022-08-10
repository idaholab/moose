//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExecFlagRegistryErrorTest.h"

#include "ExecFlagRegistry.h"

registerMooseObject("MooseTestApp", ExecFlagRegistryErrorTest);

InputParameters
ExecFlagRegistryErrorTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params.addParam<bool>(
      "already_registered", false, "Test registering an exec flag that is already registered");
  return params;
}

ExecFlagRegistryErrorTest::ExecFlagRegistryErrorTest(const InputParameters & params)
  : GeneralUserObject(params)
{
  if (getParam<bool>("already_registered"))
    defineExecFlag("NONE");
}
