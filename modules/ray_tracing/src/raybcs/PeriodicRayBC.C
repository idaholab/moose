//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PeriodicRayBC.h"

#include "RayTracingStudy.h"
#include "PeriodicBCHelper.h"

#include "libmesh/periodic_boundaries.h"
#include "libmesh/periodic_boundary.h"

registerMooseObject("RayTracingApp", PeriodicRayBC);

const std::string PeriodicRayBC::periodic_boundaries_param = "_periodic_boundaries";

InputParameters
PeriodicRayBC::validParams()
{
  auto params = GeneralRayBC::validParams();
  params += Moose::PeriodicBCHelper::validParams();

  // To be filled with the PeriodicBoundaries object from the
  // SetupPeriodicRayBCAction, and is also used to identify
  // before construction that this object is a PeriodicRayBC
  params.addPrivateParam<const libMesh::PeriodicBoundaries *>(periodic_boundaries_param, nullptr);

  // This is now set via auto_direction or primary and secondary
  params.suppressParameter<std::vector<BoundaryName>>("boundary");

  params.addClassDescription("A RayBC that enforces periodic boundaries.");

  return params;
}

PeriodicRayBC::PeriodicRayBC(const InputParameters & params)
  : GeneralRayBC(params),
    _periodic_boundaries(
        *getCheckedPointerParam<const libMesh::PeriodicBoundaries *>(periodic_boundaries_param)),
    _point_locator(_mesh.getPointLocator()),
    _periodic_applied(0),
    _periodic_neighbor(nullptr)
{
}

bool
PeriodicRayBC::isPeriodicRayBC(const InputParameters & params)
{
  return params.have_parameter<const libMesh::PeriodicBoundaries *>(periodic_boundaries_param);
}

void
PeriodicRayBC::onBoundary(const unsigned int num_applying)
{
  // No need to do anything if the Ray's gonna die anyway
  if (!currentRay()->shouldContinue())
    return;

  // The current side we're on, which isn't the physical
  // side we're currently at if we've applied multiple periodic
  // boundaries
  unsigned int current_side = _current_intersected_side;

  // Reset shared state between calls; needed in the event that
  // we are applying multiple periodic boundary conditions. Begin
  // out with the actual intersection information (the neighbor
  // is cleared once all boundaries are applied for a single Ray)
  if (!_periodic_neighbor)
  {
    _periodic_point = _current_intersection_point;
    _periodic_neighbor = _current_elem;
  }
  // We do have a periodic neighbor, which means we're applying
  // more than one periodic boundary and need to find the side
  // from the last topological neighbor
  else
  {
    mooseAssert(num_applying > 1, "Should be applying multiple");
    current_side = _mesh.sideWithBoundaryID(_periodic_neighbor, _current_bnd_id);
  }

  // The PeriodicBoundary object associated with the periodic boundary
  const auto periodic_boundary = _periodic_boundaries.boundary(_current_bnd_id);
  if (!periodic_boundary)
    mooseError("Failed to find periodic boundary\n\n", currentRay()->getInfo());

  // Move the point; this translation could happen multiple times if we are
  // applying more than one periodic boundary, which is why the state
  // is shared outside of onBoundary()
  _periodic_point = periodic_boundary->get_corresponding_pos(_periodic_point);

  // Find the topological neighbor
  unsigned int periodic_side;
  _periodic_neighbor = _periodic_boundaries.neighbor(
      _current_bnd_id, *_point_locator, _periodic_neighbor, current_side, &periodic_side);
  mooseAssert(_periodic_neighbor, "Should be set");
  if (_periodic_neighbor == libMesh::remote_elem)
    mooseError(
        "Topological neighbor at ", _periodic_point, " is remote\n\n", currentRay()->getInfo());

  // We've done all translations, so actually move the Ray
  if (++_periodic_applied == num_applying)
  {
    currentRay()->changePointElemSide(_periodic_point, *_periodic_neighbor, periodic_side, {});
    _periodic_neighbor = nullptr;
    _periodic_applied = 0;
  }
}
