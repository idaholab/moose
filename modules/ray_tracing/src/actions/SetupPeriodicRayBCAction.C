//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SetupPeriodicRayBCAction.h"

#include "AddRayBCAction.h"
#include "PeriodicRayBC.h"

registerMooseAction("RayTracingApp", SetupPeriodicRayBCAction, "add_geometric_rm");
registerMooseAction("RayTracingApp", SetupPeriodicRayBCAction, "add_periodic_bc");

InputParameters
SetupPeriodicRayBCAction::validParams()
{
  auto params = Action::validParams();
  params.addClassDescription("Sets up the periodic boundaries for a PeriodicRayBC if applicable.");
  return params;
}

SetupPeriodicRayBCAction::SetupPeriodicRayBCAction(const InputParameters & params)
  : Action(params),
    Moose::PeriodicBCHelper(getAddRayBCAction(), /* algebraic = */ false),
    _is_periodic_ray_bc(PeriodicRayBC::isPeriodicRayBC(getAddRayBCAction().getObjectParams()))
{
  if (_is_periodic_ray_bc)
    checkPeriodicParams();
}

void
SetupPeriodicRayBCAction::act()
{
  if (!_is_periodic_ray_bc)
    return;

  // Tell the mesh to hold off on deleting remote elements because we need to wait for our
  // periodic boundaries to be added
  if (_current_task == "add_geometric_rm")
    _mesh->allowRemoteElementRemoval(false);
  else if (_current_task == "add_periodic_bc")
    setupPeriodicBoundaries(*_problem);
}

void
SetupPeriodicRayBCAction::setupPeriodicRayBC(InputParameters & params) const
{
  mooseAssert(_is_periodic_ray_bc, "Not a PeriodicRayBC");

  const auto & periodic_boundaries = getPeriodicBoundaries();

  // Form boundaries for hidden "boundary" param
  std::vector<BoundaryName> boundary;
  for (const auto & it : periodic_boundaries)
    boundary.push_back(std::to_string(it.first));
  params.set<std::vector<BoundaryName>>("boundary") = boundary;

  // Allow BC to access the PeriodicBoundaries object
  params.set<const libMesh::PeriodicBoundaries *>(PeriodicRayBC::periodic_boundaries_param) =
      &periodic_boundaries;
}

const AddRayBCAction &
SetupPeriodicRayBCAction::getAddRayBCAction() const
{
  return _app.actionWarehouse().getAction<AddRayBCAction>(name());
}
