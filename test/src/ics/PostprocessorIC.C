//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PostprocessorIC.h"

registerMooseObject("MooseTestApp", PostprocessorIC);

InputParameters
PostprocessorIC::validParams()
{
  InputParameters params = InitialCondition::validParams();
  params.addClassDescription("This initial condition takes values from post-processor values.");
  params.addRequiredParam<PostprocessorName>("pp1", "Name of first test post-processor");
  return params;
}

PostprocessorIC::PostprocessorIC(const InputParameters & parameters)
  : InitialCondition(parameters),
    _pp1(getPostprocessorValue("pp1")),
    _pp2(getPostprocessorValueByName("pp2"))
{
}

Real
PostprocessorIC::value(const Point & /*p*/)
{
  return _pp1 + _pp2;
}
