//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KillRayKernel.h"

registerMooseObject("RayTracingApp", KillRayKernel);

InputParameters
KillRayKernel::validParams()
{
  auto params = GeneralRayKernel::validParams();
  params.addClassDescription("A RayKernel that kills a Ray.");
  return params;
}

KillRayKernel::KillRayKernel(const InputParameters & params) : GeneralRayKernel(params) {}

void
KillRayKernel::onSegment()
{
  currentRay()->setShouldContinue(false);
}
