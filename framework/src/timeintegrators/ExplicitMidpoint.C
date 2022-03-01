//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ExplicitMidpoint.h"

registerMooseObject("MooseApp", ExplicitMidpoint);

InputParameters
ExplicitMidpoint::validParams()
{
  InputParameters params = ExplicitRK2::validParams();
  params.addClassDescription("Time integration using the explicit midpoint method.");

  return params;
}

ExplicitMidpoint::ExplicitMidpoint(const InputParameters & parameters) : ExplicitRK2(parameters) {}
