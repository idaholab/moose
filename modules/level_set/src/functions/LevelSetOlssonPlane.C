//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetOlssonPlane.h"
#include "libmesh/utility.h"

registerMooseObject("LevelSetApp", LevelSetOlssonPlane);

InputParameters
LevelSetOlssonPlane::validParams()
{
  InputParameters params = Function::validParams();
  params.addClassDescription("Implementation of a level set function to represent a plane.");
  params.addParam<RealVectorValue>("point", RealVectorValue(0, 0, 0), "A point on the plane.");
  params.addParam<RealVectorValue>(
      "normal", RealVectorValue(0, 1, 0), "The normal vector to the plane.");
  params.addParam<Real>("epsilon", 0.01, "The interface thickness.");
  return params;
}

LevelSetOlssonPlane::LevelSetOlssonPlane(const InputParameters & parameters)
  : Function(parameters),
    _point(getParam<RealVectorValue>("point")),
    _normal(getParam<RealVectorValue>("normal")),
    _epsilon(getParam<Real>("epsilon"))
{
}

Real
LevelSetOlssonPlane::value(Real /*t*/, const Point & p) const
{
  const RealVectorValue unit_normal = _normal / _normal.norm();
  const Real distance_from_orgin = -unit_normal * _point;
  const Real x = -(unit_normal * p + distance_from_orgin) / _epsilon;

  return 1.0 / (1 + std::exp(x));
}

RealGradient
LevelSetOlssonPlane::gradient(Real /*t*/, const Point & p) const
{
  const RealVectorValue unit_normal = _normal / _normal.norm();
  const Real distance_from_orgin = -unit_normal * _point;
  const Real x = -(unit_normal * p + distance_from_orgin) / _epsilon;

  RealGradient output;
  Real x_prime;

  for (const auto i : make_range(Moose::dim))
  {
    x_prime = -unit_normal(i) / _epsilon;
    output(i) = -(x_prime * std::exp(x)) / Utility::pow<2>(std::exp(x) + 1);
  }

  return output;
}
