//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReporterInterfaceErrorTest.h"

registerMooseObject("MooseTestApp", ReporterInterfaceErrorTest);

InputParameters
ReporterInterfaceErrorTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addParam<ReporterName>("reporter", "Test parameter for a ReporterName");

  params.addParam<bool>("missing_parameter", false, "Test the error for a missing parameter");
  params.addParam<bool>("bad_parameter_type", false, "Test the error for a bad parameter type");
  params.addParam<bool>(
      "other_type_requested",
      false,
      "True to test the error for the Reporter value being registered with another type");
  params.addParam<bool>("missing",
                        false,
                        "Test the error after reporters are added and requesting a reporter value "
                        "that does not exist");
  params.addParam<bool>("missing_by_name",
                        false,
                        "Test the error after reporters are added and requesting a reporter value "
                        "by name that does not exist");
  params.addParam<bool>(
      "has_early", false, "Test the error for seeing if a Reporter exists too early");
  params.addParam<bool>("has_early_by_name",
                        false,
                        "Test the error for seeing if a Reporter exists by name too early");
  return params;
}

ReporterInterfaceErrorTest::ReporterInterfaceErrorTest(const InputParameters & params)
  : GeneralUserObject(params)
{
  if (getParam<bool>("missing_parameter"))
    getReporterValue<int>("bad_param");
  if (getParam<bool>("bad_parameter_type"))
    getReporterValue<int>("missing_parameter");
  if (getParam<bool>("other_type_requested"))
  {
    getReporterValueByName<Real>("some_reporter/some_value");
    getReporterValueByName<int>("some_reporter/some_value");
  }
  if (getParam<bool>("has_early"))
    hasReporterValue<Real>("reporter");
  if (getParam<bool>("has_early_by_name"))
    hasReporterValueByName<Real>("some_reporter/some_value");
}

void
ReporterInterfaceErrorTest::initialSetup()
{
  if (getParam<bool>("missing"))
    getReporterValue<int>("reporter");
  if (getParam<bool>("missing_by_name"))
    getReporterValueByName<int>("some_reporter/some_value");
}
