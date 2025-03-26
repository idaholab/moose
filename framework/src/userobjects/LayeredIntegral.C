//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LayeredIntegral.h"

#include "libmesh/mesh_tools.h"

registerMooseObject("MooseApp", LayeredIntegral);

InputParameters
LayeredIntegral::validParams()
{
  auto params = LayeredIntegralBase<ElementIntegralVariableUserObject>::validParams();
  params.addClassDescription("Compute variable integrals over layers.");
  return params;
}

LayeredIntegral::LayeredIntegral(const InputParameters & parameters)
  : LayeredIntegralBase<ElementIntegralVariableUserObject>(parameters)
{
}
