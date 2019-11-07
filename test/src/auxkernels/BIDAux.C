//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BIDAux.h"

registerMooseObject("MooseTestApp", BIDAux);

InputParameters
BIDAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  return params;
}

BIDAux::BIDAux(const InputParameters & parameters) : AuxKernel(parameters) {}

Real
BIDAux::computeValue()
{
  return _current_boundary_id;
}
