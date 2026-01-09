//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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
  params.addClassDescription("Sums the values of several postprocessors");
  params.addRequiredParam<std::vector<PostprocessorName>>("values",
                                                          "List of postprocessors to add");
  return params;
}

SumPostprocessor::SumPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters)
{
  const std::vector<PostprocessorName> & pps_names =
      getParam<std::vector<PostprocessorName>>("values");
  for (auto & name : pps_names)
    _values.push_back(&getPostprocessorValueByName(name));
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
SumPostprocessor::getValue() const
{
  Real sum = 0;
  for (auto & v : _values)
    sum += *v;
  return sum;
}
