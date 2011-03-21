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

#include "SphereFunction.h"

template<>
InputParameters validParams<SphereFunction>()
{
  InputParameters params = validParams<Function>();
  params.addRequiredParam<Real>("x_center", "The x coordinate of the center of the sphere");
  params.addRequiredParam<Real>("y_center", "The y coordinate of the center of the sphere");
  params.addParam<Real>("z_center", 0.0, "The z coordinate of the center of the sphere");
  params.addRequiredParam<Real>("radius", "The radius of the Sphere");
  params.addParam<Real>("inside", 1.0, "The value to be returned when the point lies within the sphere");
  params.addParam<Real>("outside", 0.0, "The value to be returned when the point lies within the sphere");
  return params;
}

SphereFunction::SphereFunction(const std::string & name, InputParameters parameters) :
  Function(name, parameters),
  _center(getParam<Real>("x_center"), getParam<Real>("y_center"), getParam<Real>("z_center")),
  _radius(getParam<Real>("radius")),
  _inside(getParam<Real>("inside")),
  _outside(getParam<Real>("outside"))
{
}

Real
SphereFunction::value(Real, const Point &p)
{
  if (p(0)-_center(0)*p(0)-_center(0) + p(1)-_center(1)*p(1)-_center(1) + p(2)-_center(2)*p(2)-_center(2) < _radius*_radius)
    return _inside;
  else
    return _outside;
}
