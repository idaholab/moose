//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorInterfaceErrorTest.h"

registerMooseObject("MooseTestApp", PostprocessorInterfaceErrorTest);

InputParameters
PostprocessorInterfaceErrorTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addParam<PostprocessorName>("pp", 1, "Test parameter for a PostprocessorName");
  params.addParam<std::vector<PostprocessorName>>(
      "pps", "Test parameter for a vector of PostprocessorNames");
  params.addParam<bool>(
      "missing_parameter", false, "True to test the error for a missing parameter");
  params.addParam<bool>(
      "bad_parameter_type", false, "True to test the error for a bad parameter type");
  params.addParam<bool>(
      "out_of_range_single", false, "Test the error for accessing a single pp out of range");
  params.addParam<bool>("out_of_range_vector",
                        false,
                        "Test the error for accessing a vector of postprocessors out of range");
  params.addParam<bool>("name_for_default",
                        false,
                        "Test the error for getting the name of a pp that is a default value");
  params.addParam<bool>("has_early", false, "Test the error for seeing if a pp exists too early");
  params.addParam<bool>(
      "has_early_by_name", false, "Test the error for seeing if a pp exists by name too early");

  params.addParam<bool>(
      "missing",
      false,
      "Test the error after pps are added and coupling to a pp that does not exist");
  params.addParam<bool>(
      "missing_by_name",
      false,
      "Test the error after pps are added and coupling to a pp by name that does not exist");

  return params;
}

PostprocessorInterfaceErrorTest::PostprocessorInterfaceErrorTest(const InputParameters & params)
  : GeneralUserObject(params)
{
  if (getParam<bool>("missing_parameter"))
    getPostprocessorValue("bad_parameter");
  if (getParam<bool>("bad_parameter_type"))
    getPostprocessorValue("bad_parameter_type");
  if (getParam<bool>("out_of_range_single"))
    getPostprocessorValue("pp", 1);
  if (getParam<bool>("out_of_range_vector"))
    getPostprocessorValue("pps", 100);
  if (getParam<bool>("name_for_default"))
    getPostprocessorName("pps", 1);
  if (getParam<bool>("has_early"))
    hasPostprocessor("dummy");
  if (getParam<bool>("has_early_by_name"))
    hasPostprocessorByName("dummy");
}

void
PostprocessorInterfaceErrorTest::initialSetup()
{
  // These methods will only report errors after the add_postprocessor action
  // has been exeucted (pps have been constructed). Otherwise, errors will be reported
  // once the Reporters are finalized and checked
  mooseAssert(getMooseApp().actionWarehouse().isTaskComplete("add_postprocessor"),
              "Assumption that pp task is complete is invalid");

  if (getParam<bool>("missing"))
    getPostprocessorValue("pp");
  if (getParam<bool>("missing_by_name"))
    getPostprocessorValueByName("missing_pp");
}
