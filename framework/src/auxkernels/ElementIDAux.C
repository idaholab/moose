//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementIDAux.h"

registerMooseObject("MooseApp", ElementIDAux);

InputParameters
ElementIDAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Displays the current mesh element ID.");
  return params;
}

ElementIDAux::ElementIDAux(const InputParameters & parameters) : AuxKernel(parameters) {}

Real
ElementIDAux::computeValue()
{
  return _current_elem->id();
}
