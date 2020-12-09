//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NullRayBC.h"

registerMooseObject("RayTracingApp", NullRayBC);

InputParameters
NullRayBC::validParams()
{
  auto params = GeneralRayBC::validParams();
  params.addClassDescription("A RayBC that does nothing to a Ray on a boundary.");
  return params;
}

NullRayBC::NullRayBC(const InputParameters & params) : GeneralRayBC(params) {}

void
NullRayBC::onBoundary(const unsigned int /* num_applying */)
{
}
