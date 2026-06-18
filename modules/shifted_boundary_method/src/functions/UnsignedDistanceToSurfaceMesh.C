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

registerMooseObject("ShiftedBoundaryMethodApp", UnsignedDistanceToSurfaceMesh);

InputParameters
UnsignedDistanceToSurfaceMesh::validParams()
{
  InputParameters params = Function::validParams();
  params.addRequiredParam<UserObjectName>(
      "builder", "SBMSurfaceMeshBuilder that provides KDTree, elem_id_map, and boundary elements.");

  params.addClassDescription(
      "Returns unsigned distance to a surface mesh using KDTree nearest neighbor search. "
      "The gradient returns the unit vector pointing from boundary to query point.");
  return params;
}

UnsignedDistanceToSurfaceMesh::UnsignedDistanceToSurfaceMesh(const InputParameters & parameters)
  : Function(parameters)
{
}

void
UnsignedDistanceToSurfaceMesh::initialSetup()
{
  const auto builder = &getUserObject<SBMSurfaceMeshBuilder>("builder");

  if (!builder->hasKDTree())
    mooseError("UnsignedDistanceToSurfaceMesh '",
               name(),
               "' requires SBMSurfaceMeshBuilder '",
               builder->name(),
               "' to be configured with 'build_kd_tree = true'.");

  _kd_tree = &builder->getKDTree();
  _boundary_elements = &builder->getBoundaryElements();
}

const SBMBndElementBase &
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
  const SBMBndElementBase & elem = closestBoundaryElem(p);
  return elem.distanceFrom(p);
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
  const SBMBndElementBase & elem = closestBoundaryElem(p);

  RealVectorValue n = elem.normal();
  const Real n_norm = n.norm();

  if (n_norm <= libMesh::TOLERANCE)
    return RealVectorValue(0, 0, 0);

  return n / n_norm;
}
