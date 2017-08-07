/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "GeometrySphere.h"

template <>
InputParameters
validParams<GeometrySphere>()
{
  InputParameters params = validParams<GeometryBase>();
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
