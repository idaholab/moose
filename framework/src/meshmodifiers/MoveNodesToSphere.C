//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MoveNodesToSphere.h"

registerMooseObject("MooseApp", MoveNodesToSphere);
registerMooseObjectRenamed("MooseApp", GeometrySphere, "06/30/2025 24:00", MoveNodesToSphere);

InputParameters
MoveNodesToSphere::validParams()
{
  InputParameters params = MoveNodesToGeometryModifierBase::validParams();
  params.addClassDescription(
      "Snap nodes to the surface of a sphere either during adaptivity or on execution");
  params.addParam<Point>("center", "Sphere center");
  params.addParam<Real>("radius", "Sphere radius");
  return params;
}

MoveNodesToSphere::MoveNodesToSphere(const InputParameters & parameters)
  : MoveNodesToGeometryModifierBase(parameters),
    _center(getParam<Point>("center")),
    _radius(getParam<Real>("radius"))
{
}

void
MoveNodesToSphere::snapNode(Node & node)
{
  const Point o = node - _center;
  const Real r = o.norm();
  node = o * _radius / r + _center;
}
