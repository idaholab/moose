//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "GeometrySphere.h"

registerMooseObject("MooseApp", GeometrySphere);

InputParameters
GeometrySphere::validParams()
{
  InputParameters params = GeometryBase::validParams();
  params.addClassDescription("Snap nodes to the surface of a sphere on adaptivity");
  params.addParam<Point>("center", "Sphere center");
  params.addParam<Real>("radius", "Sphere radius");
  return params;
}

GeometrySphere::GeometrySphere(const InputParameters & parameters)
  : GeometryBase(parameters), _center(getParam<Point>("center")), _radius(getParam<Real>("radius"))
{
}

void
GeometrySphere::snapNode(Node & node)
{
  const Point o = node - _center;
  const Real r = o.norm();
  node = o * _radius / r + _center;
}
