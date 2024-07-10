//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "WebServerControlTestReporter.h"

registerMooseObject("MooseTestApp", WebServerControlTestReporter);

InputParameters
WebServerControlTestReporter::validParams()
{
  auto params = GeneralReporter::validParams();
  params.addParam<bool>("bool_value", "Test bool value");
  params.addParam<Real>("real_value", "Test Real value");
  params.addParam<int>("int_value", "Test int value");
  params.addParam<std::vector<Real>>("vec_real_value", "Test vector Real value");
  params.addParam<std::vector<int>>("vec_int_value", "Test vector int value");
  params.addParam<std::string>("string_value", "Test string value");
  params.addParam<std::vector<std::string>>("vec_string_value", "Test vector string value");
  params.declareControllable(
      "bool_value real_value int_value vec_real_value vec_int_value string_value vec_string_value");
  return params;
}

WebServerControlTestReporter::WebServerControlTestReporter(const InputParameters & parameters)
  : GeneralReporter(parameters)
{
  declare<bool>("bool_value");
  declare<Real>("real_value");
  declare<int>("int_value");
  declare<std::vector<Real>>("vec_real_value");
  declare<std::vector<int>>("vec_int_value");
  declare<std::string>("string_value");
  declare<std::vector<std::string>>("vec_string_value");
}

void
WebServerControlTestReporter::execute()
{
  for (auto & value_ptr : _values)
    value_ptr->update();
}
