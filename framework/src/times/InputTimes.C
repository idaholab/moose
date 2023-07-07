//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InputTimes.h"

registerMooseObject("MooseApp", InputTimes);

InputParameters
InputTimes::validParams()
{
  InputParameters params = Times::validParams();
  params.addClassDescription("Times set directly from a user parameter in the input file");
  params.addRequiredParam<std::vector<Real>>("times", "Times to store in the times vector");

  // Times are known for all processes already
  params.set<bool>("auto_broadcast") = false;

  return params;
}

InputTimes::InputTimes(const InputParameters & parameters) : Times(parameters)
{
  _times = getParam<std::vector<Real>>("times");
}
