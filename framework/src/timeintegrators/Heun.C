//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Heun.h"

registerMooseObject("MooseApp", Heun);

InputParameters
Heun::validParams()
{
  InputParameters params = ExplicitRK2::validParams();
  params.addClassDescription("Heun's (aka improved Euler) time integration method.");
  return params;
}

Heun::Heun(const InputParameters & parameters) : ExplicitRK2(parameters) {}
