//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddRayBCAction.h"

#include "RayBoundaryConditionBase.h"
#include "PeriodicRayBC.h"
#include "SetupPeriodicRayBCAction.h"

registerMooseAction("RayTracingApp", AddRayBCAction, "add_ray_boundary_condition");

InputParameters
AddRayBCAction::validParams()
{
  auto params = AddRayTracingObjectAction::validParams();
  params.addClassDescription("Adds a RayBC for use in ray tracing to the simulation.");
  return params;
}

AddRayBCAction::AddRayBCAction(const InputParameters & params) : AddRayTracingObjectAction(params)
{
}

void
AddRayBCAction::addRayTracingObject()
{
  // Special case for the PeriodicRayBC, which needs to have geometric
  // ghosting and periodic boundaries setup before the object is constructed.
  // A SetupPeriodicRayBCAction will exist for every RayBC, and if this
  // is a PeriodicRayBC, will setup the periodic boundaries early
  if (PeriodicRayBC::isPeriodicRayBC(_moose_object_pars))
    _app.actionWarehouse().getAction<SetupPeriodicRayBCAction>(name()).setupPeriodicRayBC(
        _moose_object_pars);

  _problem->addObject<RayBoundaryConditionBase>(_type, _name, _moose_object_pars);
}
