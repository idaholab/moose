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

  _kd_tree = const_cast<KDTree *>(&builder->getKDTree());
  _elem_id_map = &builder->getElemIdMap();
  _boundary_elements = &builder->getBoundaryElements();

  mooseAssert(_kd_tree, "UnsignedDistanceToSurfaceMesh: KDTree is null");
  mooseAssert(_elem_id_map, "UnsignedDistanceToSurfaceMesh: elem_id_map is null");
  mooseAssert(_boundary_elements, "UnsignedDistanceToSurfaceMesh: boundary_elements is null");
}

SBMBndElementBase &
UnsignedDistanceToSurfaceMesh::closestBoundaryElem(const Point & p) const
{
  // KDTree nearest neighbor search
  std::vector<std::size_t> ret_index(1);
  _kd_tree->neighborSearch(p, 1, ret_index);

  return *_boundary_elements->at(_elem_id_map->at(ret_index[0])).get();
}

RealVectorValue
UnsignedDistanceToSurfaceMesh::distanceVectorToSurface(const Point & p) const
{
  SBMBndElementBase & elem = closestBoundaryElem(p);
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

  return dv / dist;
}

RealVectorValue
UnsignedDistanceToSurfaceMesh::surfaceNormal(const Point & p) const
{
  SBMBndElementBase & elem = closestBoundaryElem(p);

  RealVectorValue n = elem.normal();
  const Real n_norm = n.norm();

  if (n_norm <= libMesh::TOLERANCE)
    return RealVectorValue(0, 0, 0);

  return n / n_norm;
}
