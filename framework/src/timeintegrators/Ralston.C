//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Ralston.h"

registerMooseObject("MooseApp", Ralston);

InputParameters
Ralston::validParams()
{
  InputParameters params = ExplicitRK2::validParams();
  params.addClassDescription("Ralston's time integration method.");
  return params;
}

Ralston::Ralston(const InputParameters & parameters) : ExplicitRK2(parameters) {}
