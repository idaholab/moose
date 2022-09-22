//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddRayKernelAction.h"

// Local includes
#include "RayKernelBase.h"

registerMooseAction("RayTracingApp", AddRayKernelAction, "add_ray_kernel");

InputParameters
AddRayKernelAction::validParams()
{
  auto params = AddRayTracingObjectAction::validParams();
  params.addClassDescription("Adds a RayKernel for use in ray tracing to the simulation.");
  return params;
}

AddRayKernelAction::AddRayKernelAction(const InputParameters & params)
  : AddRayTracingObjectAction(params)
{
}

void
AddRayKernelAction::addRayTracingObject()
{
  _problem->addObject<RayKernelBase>(_type, _name, _moose_object_pars);
}
