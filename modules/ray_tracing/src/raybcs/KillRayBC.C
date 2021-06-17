//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KillRayBC.h"

registerMooseObject("RayTracingApp", KillRayBC);

InputParameters
KillRayBC::validParams()
{
  auto params = GeneralRayBC::validParams();
  params.addClassDescription("A RayBC that kills a Ray on a boundary.");
  return params;
}

KillRayBC::KillRayBC(const InputParameters & params) : GeneralRayBC(params) {}

void
KillRayBC::onBoundary(const unsigned int /* num_applying */)
{
  // After RayBCs are completed, ray->shouldContinue() is checked and this will kill the Ray
  currentRay()->setShouldContinue(false);
}
