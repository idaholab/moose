//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SphereContactor.h"

registerMooseObject("ContactApp", SphereContactor);

InputParameters
SphereContactor::validParams()
{
  InputParameters params = LevelSetContactor::validParams();
  params.addClassDescription("Rigid spherical contactor.");
  params.addRequiredParam<Point>("center", "Sphere center.");
  params.addRequiredRangeCheckedParam<Real>("radius", "radius > 0", "Sphere radius.");
  return params;
}

SphereContactor::SphereContactor(const InputParameters & p)
  : LevelSetContactor(p), _center(getParam<Point>("center")), _radius(getParam<Real>("radius"))
{
}

Real
SphereContactor::signedDistanceRaw(const Point & x) const
{
  return (x - _center).norm() - _radius;
}

RealVectorValue
SphereContactor::normalRaw(const Point & x) const
{
  const RealVectorValue r = x - _center;
  const Real rn = r.norm();
  if (rn == 0.0)
    return RealVectorValue(0.0, 0.0, 1.0);
  return r / rn;
}

RealTensorValue
SphereContactor::hessianRaw(const Point & x) const
{
  const RealVectorValue r = x - _center;
  const Real rn = r.norm();
  if (rn == 0.0)
    return RealTensorValue();
  const RealVectorValue n = r / rn;
  RealTensorValue H;
  for (unsigned int i = 0; i < LIBMESH_DIM; ++i)
    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
      H(i, j) = ((i == j ? 1.0 : 0.0) - n(i) * n(j)) / rn;
  return H;
}
