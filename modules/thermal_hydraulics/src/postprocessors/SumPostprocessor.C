//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SumPostprocessor.h"

registerMooseObject("ThermalHydraulicsApp", SumPostprocessor);

InputParameters
SumPostprocessor::validParams()
{
  InputParameters params = GeneralPostprocessor::validParams();
  params.addParam<std::vector<PostprocessorName>>("values", "List of postprocessors to add");
  params.addDeprecatedParam<PostprocessorName>("a", "First postprocessor", "Use 'values' instead.");
  params.addDeprecatedParam<PostprocessorName>(
      "b", "Second postprocessor", "Use 'values' instead.");

  return params;
}

SumPostprocessor::SumPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
  if (isParamValid("a") && isParamValid("b"))
  {
    _values.push_back(&getPostprocessorValue("a"));
    _values.push_back(&getPostprocessorValue("b"));
  }
  else if (isParamValid("values"))
  {
    const std::vector<PostprocessorName> & pps_names =
        getParam<std::vector<PostprocessorName>>("values");
    for (auto & name : pps_names)
      _values.push_back(&getPostprocessorValueByName(name));
  }
  else
    mooseError("Use 'values' parameter to specify postprocessor names that will be added up.");
}

void
SumPostprocessor::initialize()
{
}

void
SumPostprocessor::execute()
{
}

PostprocessorValue
SumPostprocessor::getValue()
{
  Real sum = 0;
  for (auto & v : _values)
    sum += *v;
  return sum;
}
