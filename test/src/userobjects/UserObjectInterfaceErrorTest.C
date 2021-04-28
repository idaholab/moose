//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UserObjectInterfaceErrorTest.h"

#include "ThreadedGeneralUserObject.h"

registerMooseObject("MooseTestApp", UserObjectInterfaceErrorTest);

InputParameters
UserObjectInterfaceErrorTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addParam<UserObjectName>("uo", "Test parameter for a UserObjectName");

  params.addParam<bool>(
      "missing_parameter", false, "True to test the error for a missing parameter");
  params.addParam<bool>(
      "bad_parameter_type", false, "True to test the error for a bad parameter type");
  params.addParam<bool>(
      "not_found_by_param",
      false,
      "True to test the error for a missing UO when the name is provided via paramater");
  params.addParam<bool>(
      "not_found_by_name",
      false,
      "True to test the error for a missing UO when the name is directly provided");
  params.addParam<bool>(
      "bad_cast", false, "True to test the error for a UO that fails the cast via parameter");
  params.addParam<bool>(
      "bad_cast_by_name", false, "True to test the error for a UO that fails the cast via name");

  return params;
}

UserObjectInterfaceErrorTest::UserObjectInterfaceErrorTest(const InputParameters & params)
  : GeneralUserObject(params)
{
  if (getParam<bool>("missing_parameter"))
    getUserObjectBase("missing");
  if (getParam<bool>("bad_parameter_type"))
    getUserObjectBase("bad_parameter_type");
  if (getParam<bool>("not_found_by_param"))
    getUserObjectBase("uo");
  if (getParam<bool>("not_found_by_name"))
    getUserObjectBaseByName("not_found_by_name");
  if (getParam<bool>("bad_cast"))
    getUserObject<ThreadedGeneralUserObject>("uo");
  if (getParam<bool>("bad_cast_by_name"))
    getUserObjectByName<ThreadedGeneralUserObject>("other_uo");
}
