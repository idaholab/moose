//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredSideIntegral.h"

registerMooseObject("MooseApp", LayeredSideIntegral);

InputParameters
LayeredSideIntegral::validParams()
{
  InputParameters params = LayeredIntegralBase<SideIntegralVariableUserObject>::validParams();
  params.addClassDescription("Computes surface integral of a variable storing partial sums for the "
                             "specified number of intervals in a direction (x,y,z).");
  return params;
}

LayeredSideIntegral::LayeredSideIntegral(const InputParameters & parameters)
  : LayeredIntegralBase<SideIntegralVariableUserObject>(parameters)
{
}
