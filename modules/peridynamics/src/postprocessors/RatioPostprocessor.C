//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RatioPostprocessor.h"

registerMooseObject("PeridynamicsApp", RatioPostprocessor);

template <>
InputParameters
validParams<RatioPostprocessor>()
{
  InputParameters params = validParams<GeneralPostprocessor>();
  params.addClassDescription(
      "Class for computing the ratio of two values calculated using two other postprocessors");

  params.addRequiredParam<PostprocessorName>(
      "dividend", "Name of postprocessor to calculate value for dividend");
  params.addRequiredParam<PostprocessorName>(
      "divisor", "Name of postprocessor to calculate value for divisor");

  return params;
}

RatioPostprocessor::RatioPostprocessor(const InputParameters & parameters)
  : GeneralPostprocessor(parameters),
    _dividend(getPostprocessorValue("dividend")),
    _divisor(getPostprocessorValue("divisor"))
{
}

PostprocessorValue
RatioPostprocessor::getValue()
{
  return _dividend / _divisor;
}
