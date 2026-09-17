//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "InfiniteCylinderContactor.h"

registerMooseObject("ContactApp", InfiniteCylinderContactor);

InputParameters
InfiniteCylinderContactor::validParams()
{
  InputParameters params = LevelSetContactor::validParams();
  params.addClassDescription(
      "Rigid contactor for an infinite circular cylinder of the given radius, "
      "whose axis passes through `origin` in the direction `axis`.");
  params.addRequiredParam<Point>("origin", "Any point on the cylinder's axis.");
  params.addRequiredParam<Point>(
      "axis",
      "Direction of the cylinder's axis.  Normalized internally; only the "
      "direction matters.");
  params.addRequiredRangeCheckedParam<Real>(
      "radius", "radius > 0", "Cylinder radius.");
  return params;
}

InfiniteCylinderContactor::InfiniteCylinderContactor(const InputParameters & p)
  : LevelSetContactor(p),
    _origin(getParam<Point>("origin")),
    _axis(getParam<Point>("axis")),
    _radius(getParam<Real>("radius"))
{
  const Real n = _axis.norm();
  if (n == 0.0)
    paramError("axis", "Must be a nonzero direction.");
  _axis /= n;
}

Real
InfiniteCylinderContactor::signedDistanceRaw(const Point & x) const
{
  // d_perp = (I - axis*axis^T) (x - origin) is the component of (x - origin)
  // in the plane perpendicular to the axis.  Distance-to-axis is |d_perp|.
  const RealVectorValue d = x - _origin;
  const RealVectorValue d_perp = d - (d * _axis) * _axis;
  return d_perp.norm() - _radius;
}

RealVectorValue
InfiniteCylinderContactor::normalRaw(const Point & x) const
{
  const RealVectorValue d = x - _origin;
  const RealVectorValue d_perp = d - (d * _axis) * _axis;
  const Real r = d_perp.norm();
  if (r == 0.0)
    // On-axis query is degenerate; return zero (any radial direction is valid).
    // Downstream RigidBodyNodalNCPKernel guards on gap > 0 so this branch is
    // touched only when the deformable node coincides with the axis, which is
    // an extreme configuration a well-posed input avoids.
    return RealVectorValue();
  return d_perp / r;
}

RealTensorValue
InfiniteCylinderContactor::hessianRaw(const Point & x) const
{
  const RealVectorValue d = x - _origin;
  const RealVectorValue d_perp = d - (d * _axis) * _axis;
  const Real r = d_perp.norm();
  if (r == 0.0)
    return RealTensorValue();
  const RealVectorValue e = d_perp / r;
  // H = (P - e*e^T) / r, where P = I - axis*axis^T is the projector into the
  // plane perpendicular to the axis.  See documentation for the derivation.
  RealTensorValue H;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    {
      const Real Pij = (i == j ? 1.0 : 0.0) - _axis(i) * _axis(j);
      H(i, j) = (Pij - e(i) * e(j)) / r;
    }
  return H;
}
