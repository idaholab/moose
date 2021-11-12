//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PerfGraphTest.h"

registerMooseObject("MooseTestApp", PerfGraphTest);

InputParameters
PerfGraphTest::validParams()
{
  InputParameters params = GeneralUserObject::validParams();

  params.addParam<bool>(
      "register_no_section", false, "Tests the error for registering a section without a name");
  params.addParam<bool>("register_no_live_message",
                        false,
                        "Tests the error for registering a section without a live message");
  params.addParam<bool>("section_id_missing",
                        false,
                        "Tests the error for getting a section ID from the registry for a section "
                        "that does not exist");
  params.addParam<bool>("section_info_missing",
                        false,
                        "Tests the error for getting SectionInfo from the registry for a section "
                        "that does not exist");

  return params;
}

PerfGraphTest::PerfGraphTest(const InputParameters & params) : GeneralUserObject(params)
{
  if (getParam<bool>("register_no_section"))
    moose::internal::getPerfGraphRegistry().registerSection("", 1, "foo", true);
  if (getParam<bool>("register_no_live_message"))
    moose::internal::getPerfGraphRegistry().registerSection("foo", 1, "", true);
  if (getParam<bool>("section_id_missing"))
    moose::internal::getPerfGraphRegistry().sectionID("foo");
  if (getParam<bool>("section_info_missing"))
    moose::internal::getPerfGraphRegistry().sectionInfo(1e6);
}
