//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "VectorPostprocessorInterfaceErrorTest.h"

registerMooseObject("MooseTestApp", VectorPostprocessorInterfaceErrorTest);

InputParameters
VectorPostprocessorInterfaceErrorTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addRequiredParam<VectorPostprocessorName>("vpp",
                                                   "Test parameter for a VectorPostprocessorName");
  params.addParam<Real>("real_param", 1, "Dummy Real parameter");

  params.addParam<bool>(
      "missing_parameter", false, "True to test the error for a missing parameter");
  params.addParam<bool>(
      "bad_parameter_type", false, "True to test the error for a bad parameter type");

  params.addParam<bool>(
      "has_value_early", false, "Test the error for seeing if a vpp vector exists too early");
  params.addParam<bool>("has_value_early_by_name",
                        false,
                        "Test the error for seeing if a vpp value exists by name too early");
  params.addParam<bool>("has_early", false, "Test the error for seeing if a vpp exists too early");
  params.addParam<bool>(
      "has_early_by_name", false, "Test the error for seeing if a vpp exists by name too early");

  params.addParam<bool>(
      "missing_vpp",
      false,
      "Test the error after vpps are added and coupling to a vpp that does not exist");
  params.addParam<bool>("missing_vector",
                        false,
                        "Test the error after vpps are added and coupling to a vpp that is missing "
                        "the requested vector");
  params.addParam<bool>(
      "missing_by_name",
      false,
      "Test the error after vpps are added and coupling to a vpp by name that does not exist");
  params.addParam<bool>("missing_vector_by_name",
                        false,
                        "Test the error after vpps are added and coupling to a vpp by name that is "
                        "missing the requested vector");

  return params;
}

VectorPostprocessorInterfaceErrorTest::VectorPostprocessorInterfaceErrorTest(
    const InputParameters & params)
  : GeneralUserObject(params)
{
  if (getParam<bool>("missing_parameter"))
    getVectorPostprocessorValue("bad_parameter", "dummy");
  if (getParam<bool>("bad_parameter_type"))
    getVectorPostprocessorValue("real_param", "dummy");
  if (getParam<bool>("has_value_early"))
    hasVectorPostprocessor("vpp", "dummy");
  if (getParam<bool>("has_value_early_by_name"))
    hasVectorPostprocessorByName("vpp", "dummy");
  if (getParam<bool>("has_early"))
    hasVectorPostprocessor("vpp");
  if (getParam<bool>("has_early_by_name"))
    hasVectorPostprocessorByName("vpp");
}

void
VectorPostprocessorInterfaceErrorTest::initialSetup()
{
  // These methods will only report errors after the add_vector_postprocessor action
  // has been exeucted (vpps have been constructed). Otherwise, errors will be reported
  // once the Reporters are finalized and checked
  mooseAssert(getMooseApp().actionWarehouse().isTaskComplete("add_vector_postprocessor"),
              "Assumption that vpp task is complete is invalid");

  if (getParam<bool>("missing_vpp"))
    getVectorPostprocessorValue("vpp", "dummy");
  if (getParam<bool>("missing_vector"))
    getVectorPostprocessorValue("vpp", "missing_vector");
  if (getParam<bool>("missing_by_name"))
    getVectorPostprocessorValueByName("missing_vpp", "dummy");
  if (getParam<bool>("missing_vector_by_name"))
    getVectorPostprocessorValueByName("constant_vpp", "missing_vector");
}
