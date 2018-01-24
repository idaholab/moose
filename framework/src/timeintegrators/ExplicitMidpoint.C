//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitMidpoint.h"

template <>
InputParameters
validParams<ExplicitMidpoint>()
{
  InputParameters params = validParams<ExplicitRK2>();

  return params;
}

ExplicitMidpoint::ExplicitMidpoint(const InputParameters & parameters) : ExplicitRK2(parameters) {}
