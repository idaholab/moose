//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UnsignedDistanceToSurfaceMesh.h"
#include "MooseError.h"
#include "SBMSurfaceMeshBuilder.h"
#include "SBMSurfaceDistance.h"
#include "SurfaceElement.h"
#include "SBMInterfaceManager.h"

registerMooseObject("ShiftedBoundaryMethodApp", UnsignedDistanceToSurfaceMesh);

InputParameters
UnsignedDistanceToSurfaceMesh::validParams()
{
  InputParameters params = Function::validParams();
  params.addParam<UserObjectName>(
      "builder", "SBMSurfaceMeshBuilder that provides the interface surface geometry.");
  params.addParam<UserObjectName>(
      "manager", "SBMInterfaceManager that provides geometry for multiple interfaces.");
  params.addParam<SubdomainID>("subdomain_id_1", "First subdomain defining the interface.");
  params.addParam<SubdomainID>("subdomain_id_2", "Second subdomain defining the interface.");

  params.addClassDescription(
      "Returns unsigned distance to a surface mesh using KDTree nearest neighbor search. "
      "The gradient returns the unit vector pointing from boundary to query point.");
  return params;
}

UnsignedDistanceToSurfaceMesh::UnsignedDistanceToSurfaceMesh(const InputParameters & parameters)
  : Function(parameters)
{
  const bool has_builder = isParamValid("builder");
  const bool has_manager = isParamValid("manager");
  const bool has_first = isParamValid("subdomain_id_1");
  const bool has_second = isParamValid("subdomain_id_2");
  if (has_builder == has_manager)
    paramError("builder", "Specify exactly one of 'builder' and 'manager'.");
  if (has_manager && (!has_first || !has_second))
    paramError("manager", "Manager mode requires subdomain_id_1 and subdomain_id_2.");
  if (!has_manager && (has_first || has_second))
    paramError("subdomain_id_1", "Subdomain IDs are only valid in manager mode.");
  if (has_manager)
  {
    _subdomain_pair = {getParam<SubdomainID>("subdomain_id_1"),
                       getParam<SubdomainID>("subdomain_id_2")};
    if (_subdomain_pair.first == _subdomain_pair.second)
      paramError("subdomain_id_2", "The interface subdomain IDs must be distinct.");
  }
}

void
UnsignedDistanceToSurfaceMesh::initialSetup()
{
  if (isParamValid("manager"))
  {
    _manager = &getUserObject<SBMInterfaceManager>("manager");
    if (!_manager->hasInterface(_subdomain_pair.first, _subdomain_pair.second))
      paramError("manager", "The requested interface was not detected by the manager.");
    return;
  }

  const auto builder = &getUserObject<SBMSurfaceMeshBuilder>("builder");
  if (!builder->hasKDTree())
    mooseError("UnsignedDistanceToSurfaceMesh '",
               name(),
               "' requires SBMSurfaceMeshBuilder '",
               builder->name(),
               "' to be configured with 'build_kd_tree = true'.");

  _kd_tree = &builder->getKDTree();
  _boundary_elements = &builder->surfaceElementSet().elements();
}

const SurfaceElement &
UnsignedDistanceToSurfaceMesh::closestBoundaryElem(const Point & p) const
{
  // KDTree nearest neighbor search
  std::vector<std::size_t> ret_index(1);
  _kd_tree->neighborSearch(p, 1, ret_index);

  return *_boundary_elements->at(ret_index.front()).get();
}

RealVectorValue
UnsignedDistanceToSurfaceMesh::distanceVectorToSurface(const Point & p) const
{
  if (_manager)
    return _manager->queryInterface(_subdomain_pair.first, _subdomain_pair.second, p).distance;

  const SurfaceElement & elem = closestBoundaryElem(p);
  return SBMUtils::distanceFrom(elem, p);
}

Real
UnsignedDistanceToSurfaceMesh::value(Real /*t*/, const Point & p) const
{
  return distanceVectorToSurface(p).norm();
}

RealGradient
UnsignedDistanceToSurfaceMesh::gradient(Real /*t*/, const Point & p) const
{
  const RealVectorValue dv = distanceVectorToSurface(p);
  const Real dist = dv.norm();

  if (dist <= libMesh::TOLERANCE)
    return RealGradient(0, 0, 0);

  // dv points from the query point toward the nearest surface, so -dv points away from it.
  // The gradient of a distance field points toward increasing distance (away from the surface),
  // matching the signed-distance gradient convention used by SBMUtils::distanceVectorFromFunction.
  return -dv / dist;
}

RealVectorValue
UnsignedDistanceToSurfaceMesh::surfaceNormal(const Point & p) const
{
  if (_manager)
    return _manager->queryInterface(_subdomain_pair.first, _subdomain_pair.second, p).normal;

  const SurfaceElement & elem = closestBoundaryElem(p);

  RealVectorValue n = elem.normal();
  const Real n_norm = n.norm();

  return n / n_norm;
}
