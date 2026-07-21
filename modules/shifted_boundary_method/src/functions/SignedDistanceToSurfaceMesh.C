//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SignedDistanceToSurfaceMesh.h"
#include "PointInPolyhedronCheckUO.h"

registerMooseObject("ShiftedBoundaryMethodApp", SignedDistanceToSurfaceMesh);

InputParameters
SignedDistanceToSurfaceMesh::validParams()
{
  InputParameters params = UnsignedDistanceToSurfaceMesh::validParams();

  params.addRequiredParam<UserObjectName>("in_out_test", "The name of the in-out test user object");

  params.addClassDescription(
      "Returns signed distance to a surface mesh using KDTree nearest neighbor search. "
      "The gradient returns the unit vector pointing from boundary to query point.");
  return params;
}

SignedDistanceToSurfaceMesh::SignedDistanceToSurfaceMesh(const InputParameters & parameters)
  : UnsignedDistanceToSurfaceMesh(parameters), _in_out_test(nullptr)
{
}

void
SignedDistanceToSurfaceMesh::initialSetup()
{
  // Lazy (Deffered) initialization
  UnsignedDistanceToSurfaceMesh::initialSetup();
  _in_out_test = &getUserObject<PointInPolyhedronCheckUO>("in_out_test");
}

Real
SignedDistanceToSurfaceMesh::value(Real t, const Point & p) const
{
  return UnsignedDistanceToSurfaceMesh::value(t, p) * (_in_out_test->ifInside(p) ? -1.0 : 1.0);
}
